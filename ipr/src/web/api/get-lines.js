'use strict';
/*jslint vars:true*/
module.exports = function (req, res) {
    var result = {
	    lines : req.app.server.line_status,
	    credit : req.app.server.config.credit.lines
    };
    res.end(JSON.stringify(result));
};

