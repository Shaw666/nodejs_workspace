
'use strict';
/*jslint vars:true, sub:true, node:true, nomen: true, maxlen: 160, plusplus:true, stupid:true*/

var net          = require('net');
var util         = require('util');
var EventEmitter = require('events').EventEmitter;
var Daemon       = require('./daemon.js');

function Daemon_tcp(server, line) {
    Daemon.call(this, server, line);
    this.socket_server   = null;
    this._ip             = "0.0.0.0";
    this._port           = server.config.ipr["tcp-lines"][line].port;
    this._protocol       = server.config.ipr["tcp-lines"][line].protocol;
    this.sessions        = {};
}
util.inherits(Daemon_tcp, Daemon);

Daemon_tcp.prototype.on_session_close = function (session) {
    this.log("session closed: ", session.sessionId);
    var acct = session.acct();
    if (acct && !session.closed) {
        this.emit('info-event', this, session,
                  { action : "offline",
                    acct   : acct});
    }
    delete this.sessions[session.sessionId];
};

Daemon_tcp.prototype.sessions_list = function () {
    var result = [];
    for (var sid in this.sessions) {
        if (this.sessions.hasOwnProperty(sid)) {
            var session = this.sessions[sid];
            result.push(session);
        }
    }
    return result
};
Daemon_tcp.prototype.get_sessions_of = function (acct) {
    if (typeof acct === 'number') {
        acct = acct.toString(16).toUpperCase();
        while (acct.length < 4) { acct = "0" + acct; }
    }
    var result = [];
    for (var sid in this.sessions) {
        if (this.sessions.hasOwnProperty(sid)) {
            var session = this.sessions[sid];
            var a = session.acct();
            if (a === acct) {
                result.push(session);
            }
        }
    }
    return result;
};

Daemon_tcp.prototype.on_new_acct = function (action, session) {
    var acct = session.acct();
    if (acct) {
        if (action == 'remove') {
            this.emit('info-event', this, session,
                      { action: "offline",
                        acct:   acct});
            return;
        } else if (action == 'new') {
            this.emit('info-event', this, session,
                      { action : "online",
                        acct   : acct})
        }
    }
    // clean all same sn device connection
    var sn = session.runtime.profile.sn;
    if (sn) {
        for (var sid in this.sessions) {
            if (this.sessions.hasOwnProperty(sid)) {
                var ss = this.sessions[sid];
                var addr1 = session.runtime.profile.raddr;
                var port1 = session.runtime.profile.rport;
                var addr2 = ss.runtime.profile.raddr;
                var port2 = ss.runtime.profile.rport;
                if (addr1 !== addr2 || port1 !== port2) {
                    var sn2 = ss.runtime.profile.sn;
                    if (sn2 &&  sn2 === sn) {
                        ss.closed = true;
                        this.log("kickout : ",
                                    session.sessionId,
                                    session.runtime.profile.sn);
                        ss.disconnect();
                        delete this.sessions[sid];
                    }
                }
            }
        }
    }
};

Daemon_tcp.prototype.get_all_session_status = function () {
    var result = [];
    var accts = {};
    for (var sid in this.sessions) {
        if (this.sessions.hasOwnProperty(sid)) {
            var session = this.sessions[sid];
            var acct = session.acct();
            if (acct) {
                if (accts[acct]) { continue; };
                accts[acct] = true;
                var event = {
                    session : session,
                    cid    : acct + " 18 R0B0 00 C000",
                    caller : session.sn(),
                    time   : new Date()
                }
                result.push(event);
                event = {
                    session : session,
                    cid    : acct + " 18 R3B0 00 C000",
                    caller : session.sn(),
                    time   : new Date()
                }
                result.push(event);
            }
        }
    }
    return result;
};


Daemon_tcp.prototype.new_session_id = function () {
    var s;
    s = (Math.random() * 10000).toFixed(0);
    s = s + '-' + (Math.random() * 10000).toFixed(0);
    return s;
};
Daemon_tcp.prototype.start = function () {
    this.socket_server = net.createServer({}, this.on_new_connection.bind(this));
    this.socket_server.listen(this.port(), this.ip());
};


module.exports = Daemon_tcp;
