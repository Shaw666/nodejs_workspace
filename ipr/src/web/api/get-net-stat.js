'use strict';
/*jslint vars:true*/
module.exports = function (req, res) {
    req.app.res_with_external_command(req, res, "box-net-stat");
};
