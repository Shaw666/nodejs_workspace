'use strict';
/*jslint vars:true*/
module.exports = function (req, res) {
    var index = req.query.index;
    if (!index) { index = 0; }
    var event_log = req.app.server.event_log;
    var result = {
        total : event_log.length,
        data  : []
    };
    for (var i = index; i < index + 1000; i++) {
        if (i >= event_log.length) { break; }
        result.data.push(event_log[event_log.length - i - 1]);
    }
    res.end(JSON.stringify(result));
};

