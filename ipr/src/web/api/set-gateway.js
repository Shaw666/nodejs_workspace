'use strict';
/*jslint vars:true*/
var fs  = require('fs');
var env = process.env;

function valid_boolean(value) {
    if (typeof value === "boolean") { return true; }
    if (value === 'true') { return true; }
    if (value === 'false') { return true; }
    return false;
}

function valid_number(value) {
    if (typeof value !== 'number') { return false; }
    if (value < 2000 || value > 10000) { return false; }
    return true;
}

function valid_string(value) {
    if (typeof value !== 'string') { return false; }
    if (value.length <= 0) { return false; }
    return true;
}

function valid_ip(value) {
    if (typeof value !== 'string') { return false; }
    var reg =  /^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$/;
    return reg.test(value);
}

var valid_field = {};
valid_field["gateway-enabled"]     = valid_boolean;
valid_field["dss7016-lan-ip"]      = valid_ip;
valid_field["dss7016-wan-ip"]      = valid_ip;
valid_field["dss7016-server-port"] = valid_number;
valid_field["dss7016-username"]    = valid_string;
valid_field["dss7016-password"]    = valid_string;
valid_field["things-server-ip"]    = valid_ip;
valid_field["things-server-port"]  = valid_number;

function set_gateway_enabled(enabled, req) {
    var file_path =  env.JFLAGS + "/gateway-enabled";
    var info;
    if (enabled) {
	    if (!fs.existsSync(file_path)) {
	        fs.openSync(file_path, "w");
	        info = "启用视频网关";
	        req.app.server.log(0, "admin", req.socket.remoteAddress, req.socket.remotePort, info);
            console.log(info);
	        return true;
	    }
    } else {
	    if (fs.existsSync(file_path)) {
	        fs.unlinkSync(file_path);
	        info = "禁用视频网关";
	        req.app.server.log(0, "admin", req.socket.remoteAddress, req.socket.remotePort, info);
            console.log(info);
	        return true;
	    }
    }

    return false;
}

module.exports = function (req, res) {
    var config = req.query.data;
    if (!config) {
        res.status(400).end();
        return;
    }
    var config_new;
    try {
        config_new = JSON.parse(config);
    } catch (error) {
        res.status(400).end();
        return;
    }
    console.log(config_new);
    var gateway_file = env.JSYS + "/etc/gateway.json";
    var config_old   = fs.readFileSync(gateway_file);
    try {
        config_old = JSON.parse(config_old);
    } catch (err) {
        res.status(400).end();
        return;
    }

    var fail = false;
    var reason = "";

    for (var key in config_new) {
        if (!config_new.hasOwnProperty(key)) { continue; };
        if (config_new[key] === config_old[key]) { continue; };
        var value = config_new[key];
        var func  = valid_field[key];

        if (func) {
            if (func(value)) {
                if (typeof config_old[key] === 'boolean') {
		            if ("gateway-enabled" === key) {
                        if (set_gateway_enabled(value, req)) {
			                config_old[key] = value;
			            }
		            }
                } else {
		            config_old[key] = value;
		            req.app.server.log(0, "admin", req.socket.remoteAddress, req.socket.remotePort, "修改视频网关参数");
		        }
            } else {
                console.log("[/api/set-gateway.js] error: ", key, value);
                fail = true;
                reason = key;
                break;
            }
        }
    }
    
    if (fail) { res.status(400).end(reason); return; }
    fs.writeFileSync(gateway_file, JSON.stringify(config_old, null, 4));

    var result = {
        result: "OK"
    };
    
    res.end(JSON.stringify(result));
};
