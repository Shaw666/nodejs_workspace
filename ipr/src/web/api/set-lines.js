'use strict';
/*jslint vars:true*/
var fs = require('fs');
function invalid_line_settings(config) {
    var lines = config["tcp-lines"];
    if (!lines) { return "找不到线路设置"; }

    var ports   = [];
    for (var key in lines) {
        if (!lines.hasOwnProperty(key)) { continue; }
        var line = lines[key];
        try {
            var n = parseInt(key, 10);
            if (n < 1 || n > 9) { return "线路号超出允许的范围"; }
        } catch (error) {
            return "非法线路号";
        }
        if (line.port < 2000 || line.port >= 10000) {
            return "端口超出允许的范围";
        }
        if (line.prefix && !/^[0-9]{4}$/.test(line.prefix)) {
            return "用户号前缀格式错误:" + prefix;
        }
        ports.push(line.port);
        if (["ipr-1", "ipr-2"].indexOf(line.protocol) < 0) {
            return "不支持的协议类型";
        }
    }
    ports.sort();
    for (var i = 0; i < ports.length - 1; i++) {
        if (ports[i] === ports[i + 1]) {
            return "协议端口重复";
        }
    }
    return "";
}
function config_changed(conf_new, conf_old) {
    for (var key in conf_old) {
        if (!conf_old.hasOwnProperty(key)) { continue; }
        if (conf_new[key] !== conf_old[key]) { return true; }
    }
    return false;
}
module.exports = function (req, res) {
    var lines_new = req.query.data;
    if (!lines_new) {
        res.status(400).end();
        return;
    }
    try {
        lines_new = JSON.parse(lines_new);
    } catch (error) {
        res.status(400).end();
        return;
    }
    var config_file = req.app.server.config_file;
    
    var config_old = fs.readFileSync(config_file);
    try {
        config_old = JSON.parse(config_old);
    } catch (error) {
        console.log(error);
        res.status(500).end();
        return;
    }
    var changed = false;
    var lines_old = config_old["tcp-lines"];
    for (var key in lines_new) {
        if (!lines_new.hasOwnProperty(key)) { continue; };
        var line_new = lines_new[key];
        var line_old = lines_old[key];
        if (!line_old) {
            lines_old[key] = line_new;
            changed = true;
        } else if (config_changed(line_new, line_old)) {
            changed = true;
            lines_old[key] = line_new;
        };
    }
    // check for delete
    var deleted = [];
    for (var key in lines_old) {
        if (lines_old.hasOwnProperty(key)) {
            if (!lines_new[key]) {
                deleted.push(key);
                changed = true;
            }
        }
    };
    for (var i = 0; i < deleted.length; i++) {
        delete lines_old[deleted[i]];
    }
    if (changed) {
        var error = invalid_line_settings(config_old);
        if (error) {
            console.log("[/api/set-line.js] error: ",
                        config_old, error);
            res.status(400).end(error);
            return;
        }
        console.log("final", config_old);
        fs.writeFileSync(config_file, JSON.stringify(config_old, null, 4));
    } else {
    }
    
    var result = {
        result: "OK"
    };
    
    res.end(JSON.stringify(result));
    req.app.server.log(0, "admin",
                       req.socket.remoteAddress,
                       req.socket.remotePort, "线路参数修改");
    process.exit(1);
};
