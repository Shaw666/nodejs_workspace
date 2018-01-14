'use strict';
var http = require('http');
/*jslint vars:true*/
module.exports = function (req, res) {
    var index = req.query.index || 0;
    req.app.server.get_runtime(index, function (runtime) {
        res.end(JSON.stringify(runtime));
    });
};
