'use strict';
/*jslint vars:true, sub:true, node:true, nomen: true, maxlen: 160, plusplus:true, stupid:true*/
var util = require('util');
var EventEmitter = require('events').EventEmitter;

function Daemon(server, line) {
    this.server    = server;
    this._line     = line;
    this._protocol = "unknown";
    this._port     = 0;
}
util.inherits(Daemon, EventEmitter);

Daemon.prototype.line = function () {
    return this._line;
};
Daemon.prototype.sessions_list = function () {
    return [];
};
Daemon.prototype.prefix = function () {
    return this.server.config.ipr["tcp-lines"][this._line].prefix;
};
Daemon.prototype.protocol = function () {
    return this._protocol;
};
Daemon.prototype.ip = function () {
    return this._ip;
};
Daemon.prototype.port = function () {
    return this._port;
};
Daemon.prototype.log = function () {
    if (!this.debug) { return; }
    var msg = "";
    for (var i = 0; i < arguments.length; i ++) {
        msg = msg + arguments[i] + " ";
    }
    console.log((new Date()).format("yyyy-MM-dd hh:mm:ss"),
                "[" + this.line() + ":" + this.port() + "]" + msg);
};
Daemon.prototype.get_sessions_of = function (acct) {
    return [];
};

module.exports = Daemon;
