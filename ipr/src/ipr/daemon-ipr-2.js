'use strict';
/*jslint vars:true, sub:true, node:true, nomen: true, maxlen: 160, plusplus:true, stupid:true*/

var net          = require('net');
var util         = require('util');
var EventEmitter = require('events').EventEmitter;
var Daemon_tcp   = require('./daemon-tcp.js');
var Session      = require("./session-ipr-2.js");

var IDLE_TIME = 300;

function Daemon_ipr_2(server, line) {
    Daemon_tcp.call(this, server, line);
    this.socket_server = null;
}
util.inherits(Daemon_ipr_2, Daemon_tcp);

Daemon_ipr_2.prototype.on_stat_report = function (session, report) {
    this.log("report: ", report);
    this.emit("stat-report", this, session, report);
};

Daemon_ipr_2.prototype.on_cid_event = function (session, event) {
    this.log("[cid event]", event.cid);
    this.emit("cid-event", this, session, event);
};

Daemon_ipr_2.prototype.on_new_connection = function (socket) {
    var session = new Session(this.server, this,
                              this.new_session_id(), socket);
    this.sessions[session.sessionId] = session;
    session.on('cid-event',    this.on_cid_event.bind(this));
    session.on('stat-report',  this.on_stat_report.bind(this));
    session.on('acct',         this.on_new_acct.bind(this));
    session.on('close',        this.on_session_close.bind(this));
};

Daemon_ipr_2.prototype.check_session_timeout = function () {
    var t_timeout = this.server.config.ipr["confirm-offline-time"];
    if (t_timeout === 0) {
        clearTimeout(this.check_timeout_handle);
        return;
    }
    for (var sid in this.sessions) {
        if (!this.sessions.hasOwnProperty(sid)) { continue; };
        var session = this.sessions[sid];
        session.idle_timer_counter = session.idle_timer_counter + 5;
        if (session.idle_timer_counter > t_timeout) {
            this.log(session.sessionId, "timeout, disconnect ...");
            session.close();
        }
    }
};
Daemon_ipr_2.prototype.start = function () {
    Daemon_tcp.prototype.start.call(this);
    this.check_timeout_handle = setInterval(this.check_session_timeout.bind(this), 5000);
    this.log("ipr-2 protocol daemon listen at: " + this.port());
};


module.exports = Daemon_ipr_2;


