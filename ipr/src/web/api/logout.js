'use strict';
/*jslint vars:true*/
module.exports = function (req, res) {
    req.app.server.log(0, "admin",
                       req.socket.remoteAddress,
                       req.socket.remotePort, "退出登录");
    res.clearCookie("sid");
    res.redirect('/login');
};
