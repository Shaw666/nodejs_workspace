'use strict';
/*jslint vars:true*/

module.exports = function (req, res) {
    if (req.query.key !== req.app.server.config.ipr["api-key"]) {
        res.status(403).end();
    } else if (!req.app.server.config.ipr["api-allow-reboot"]) {
        res.status(403).end();
    } else {
        var exec = require('child_process').exec;
        var cmd = 'reboot ';
        var child = exec(cmd, function (error, stdout, stderr) {
            // console.log('stdout: ' + stdout);
            // console.log('stderr: ' + stderr);
            if (error !== null) {
                req.app.server.log(0, "api",
                                   req.socket.remoteAddress,
                                   req.socket.remotePort, "系统重启失败:", stderr);
                console.log('exec error: ' + cmd + " : " + error);
                res.status(500).end();
            } else {
                req.app.server.log(0, "api",
                                   req.socket.remoteAddress,
                                   req.socket.remotePort, "系统重启");
                res.end("OK");
            }
        });
    }
};
