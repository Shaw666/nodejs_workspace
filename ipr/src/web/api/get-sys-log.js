'use strict';
/*jslint vars:true*/
module.exports = function (req, res) {
    var exec = require('child_process').exec, child;
    var file = req.app.server.SYS_LOG;
    child = exec('tail -n 100 ' + file, function (error, stdout, stderr) {
        // console.log('stdout: ' + stdout);
        // console.log('stderr: ' + stderr);
        if (error !== null) {
            console.log('exec error: ' + error);
            res.status(500).end(error);
        } else {
            var result = stdout.split("\n");
            result.reverse();
            res.end(JSON.stringify(result));
        }
    });
};

