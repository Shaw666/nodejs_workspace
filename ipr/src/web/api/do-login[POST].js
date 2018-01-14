'use strict';
/*jslint vars:true*/
var hanson = require('hanson');
var fs = require('fs');

var app = null;

function check_session_timeout() {
    if (!app) { return; }
    var sid;
    for (sid in app.sessions) {
        if (app.sessions.hasOwnProperty(sid)) {
            var t = new Date() - app.sessions[sid].time;
            if (t > app.SESSION_ALIVE_TIME) {
                app.server.log(0, "admin", app.sessions[sid].ip, app.sessions[sid].port, "超时退出");
                delete app.sessions[sid];
                console.log("session : ", sid, "deleted.");
            }
        }
    }
}

setInterval(check_session_timeout, 2000);

function new_sid() {
    if (!app) { return; }
    var i;
    var sid = "";
    while ((sid === "") || app.sessions[sid]) {
        for (i = 0; i < 10; i++) {
            sid = Math.random() * 100000000 + '0';
        }
    }
    return sid;
}

module.exports = function (req, res) {
    app = req.app;
    if (!app.sessions) { app.sessions = {}; }
    var pass = app.server.config.ipr["web-pass"];
    if (req.body.username === "admin" && req.body.password === pass) {
        req.app.server.log(0, "admin",
                           req.socket.remoteAddress,
                           req.socket.remotePort, "登录");
        console.log("set cookies");
        var sid = new_sid();
        app.sessions[sid] = {
            ip   : req.socket.remoteAddress,
            port : req.socket.remotePort,
            user : req.body.username,
            time : new Date()
        };
        res.cookie("sid", sid,  {
            maxAge: app.SESSION_ALIVE_TIME,
            httpOnly: true
        });
        res.redirect('/');
    } else {
        res.redirect('/login');
    }
};
