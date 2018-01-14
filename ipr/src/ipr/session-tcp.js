'use strict';
/*jslint vars:true, sub:true, node:true, nomen: true, maxlen: 160, plusplus:true, stupid:true*/
var util    = require('util');
var Session = require("./session.js");

var LF  = String.fromCharCode(0x0A);
var CR  = String.fromCharCode(0x0D);
var ACK = String.fromCharCode(0x06);
var NAK = String.fromCharCode(0x15);

function Session_tcp(server, daemon, sessionId, socket) {
    Session.call(this, server, daemon, sessionId);
    this.log("new connection from:", socket.remoteAddress);
    this.sessionId    = sessionId;
    this.socket       = socket;
    this.buffer       = "";
    this.closed       = false;
    this.runtime.tcp = {
        raddr : socket.remoteAddress,
        rport : socket.remotePort
    };

    this.socket.on("data",    this.on_net_data.bind(this));
    this.socket.on("error",   this.on_net_error.bind(this));
    this.socket.on("end",     this.on_net_end.bind(this));
    this.socket.on("timeout", this.on_net_timeout.bind(this));
    this.socket.on("drain",   this.on_net_drain.bind(this));
    this.socket.on("close",   this.on_net_close.bind(this));
}
util.inherits(Session_tcp, Session);

Session.prototype.ip = function () {
    var ip = this.runtime.tcp.raddr;
    if (!ip) { ip = ""; }
    return ip;
};

Session_tcp.prototype.on_net_data = function (data) {
    this.buffer = this.buffer + data.toString();
    this.do_process_buffer();
};
Session_tcp.prototype.on_net_error = function (err) {
    this.log("on_net_error", err);
    this.close();
};
Session_tcp.prototype.on_net_end = function () {
    this.log("on_net_end");
};
Session_tcp.prototype.on_net_timeout = function () {
    this.log("on_net_timeout");
};
Session_tcp.prototype.on_net_drain = function () {
    this.log("on_net_drain");
};
Session_tcp.prototype.on_net_close = function (had_error) {
    if (had_error) {
        this.log("on_net_close with error");
    } else {
        this.log("on_net_close");
    }
    if (this.socket) {
        this.socket.destroy();
        this.socket = null;
    }
    this.emit('close', this);
};

Session_tcp.prototype.send = function (data) {
    try {
        this.socket.write(data);
    } catch (error) {
        this.log(error, data);
    }
};
Session_tcp.prototype.close = function () {
    if (this.socket) {
        this.socket.end();
        this.socket.destroy();
        this.socket = null;
    }
};
module.exports = Session_tcp;
