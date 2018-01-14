'use strict';
var http = require('http');
var fs   = require('fs');
var env  = process.env;

/*jslint vars:true*/

var fields = [
    "gateway-enabled",
    "dss7016-lan-ip",
    "dss7016-wan-ip",
    "dss7016-server-port",
    "dss7016-username",
    "dss7016-password",
    "things-server-ip",
    "things-server-port"
];

var file_path =  env.JFLAGS + "/gateway-enabled";

function copy_fields(list, obj) {
    var result = {};
    for (var i = 0; i < list.length; i++) {
        var value = obj[list[i]];
        if (typeof value !== 'undefined') {
            result[list[i]] = value;
        }
    }

    result["gateway-enabled"] = fs.existsSync(file_path);

    return result;
}

module.exports = function (req, res) {
    var gateway_file = env.JSYS + "/etc/gateway.json";
    var gateway      = fs.readFileSync(gateway_file);
    var result       = copy_fields(fields, JSON.parse(gateway));
    res.end(JSON.stringify(result));
};
