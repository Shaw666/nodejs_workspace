'use strict';
/*jslint vars:true*/

var fs = require('fs');

module.exports = function (req, res) {

    var log_file = req.app.server.EVENT_LOG;

    var len = fs.statSync(log_file).size;    
    var filename = process.env.TID + '-' + (new Date()).format() + '.csv';

    var readStream = fs.createReadStream(log_file);

    res.set('Content-Type',        'application/octet-stream');
    res.set('Content-Length',      len);
    res.set('Content-Disposition', 'attachment; filename=' + filename);

    readStream.on('data', function (data) {
        data = data.toString().replace(/;/g, ',');
	    res.write(data);
    });

    readStream.on('end', function () {
	    res.end();
    });
};
