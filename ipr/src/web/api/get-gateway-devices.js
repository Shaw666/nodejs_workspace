'use strict';
var http = require('http');
var fs   = require('fs');
var env  = process.env;

/*jslint vars:true*/

module.exports = function (req, res) {
    req.app.server.get_gateway_runtime(function (runtime) {
        res.end(JSON.stringify(runtime));
    });
};
