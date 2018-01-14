'use strict';
var http = require('http');
/*jslint vars:true*/

var fields = [
    "reciver-number",
    "serial-format",
    "serial-baud",
    "panel-event-first",
    "report-event-online-offline",
    "report-status-online-offline",
    "log-online-offline",
    "confirm-offline-time",
    "allow-control-device",
    "monitor-beep-serial",
    "monitor-beep-net",
    "api-key",
    "api-allow-set-time",
    "api-allow-reboot",
    "web-port"
];
function copy_fields(list, obj) {
    var result = {};
    for (var i = 0; i < list.length; i++) {
        var value = obj[list[i]];
        if (typeof value !== 'undefined') {
            result[list[i]] = value;
        }
    }
    return result;
}

module.exports = function (req, res) {
    var result = copy_fields(fields, req.app.server.config.ipr);
    result["system-time"] = "";
    res.end(JSON.stringify(result));
};
