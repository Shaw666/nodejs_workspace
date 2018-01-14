'use strict';
/*jslint vars:true, sub:true, node:true, nomen: true, maxlen: 160, plusplus:true, stupid:true*/
var util = require('util');
var EventEmitter = require('events').EventEmitter;

var LF  = String.fromCharCode(0x0A);
var CR  = String.fromCharCode(0x0D);
var ACK = String.fromCharCode(0x06);
var NAK = String.fromCharCode(0x15);
var IDLE_TIME = 300;


function Session(server, daemon, sessionId) {
    this.server    = server;
    this.daemon    = daemon;
    this.sessionId = sessionId;
    this.uptime    = new Date();
    this.runtime = {
        profile : {
            tid   : null,
            brand : null,
            model : null,
            ver   : null,
            sn    : null
        },
        areas : {
            "01" : {
                acct : null,
                stat : "na"
            }
        },
        zones: {
        },
        flags : {
            "low-battery" : null,
            "ac-fail"     : null,
            "line-fail"   : null,
            "gprs"        : null
        },
        geo : {
            lon : null,
            lat : null
        },
        level : 10000
    };
}
util.inherits(Session, EventEmitter);

Session.prototype.set_acct = function (param) {
    this.log("acct set to :", param);
    if (!param) { return; }
    var acct;
    if (typeof param === 'string') {
        try {
            acct = parseInt(param, 16);
        } catch (err) {
            return;
        }
    }
    var new_acct_flag = false;
    var old_acct = this.acct();
    var old_acct_number = this.runtime.areas["01"].acct;
    this.runtime.areas["01"].acct  = acct;
    var new_acct = this.acct();
    this.runtime.areas["01"].acct = old_acct_number;
    
    if (!old_acct && acct) {
        new_acct_flag = true;
    } else if (old_acct && acct) {
        if (old_acct !== new_acct) {
            this.emit('acct', 'remove', this);
            new_acct_flag = true;
        }
    }
    
    this.runtime.areas["01"].acct = acct;
    
    if (new_acct_flag) {
        this.emit('acct', 'new', this);
    }
};
Session.prototype.ip = function () {
    return "";
};
Session.prototype.level = function () {
    return this.runtime.level;
};
Session.prototype.tid = function () {
    var result = this.runtime.profile.tid;
    if (!result) { result = ""; }
    return result;
};
Session.prototype.sn = function () {
    var sn = this.runtime.profile.sn;
    if (sn) { return sn; };
    var tid = this.runtime.profile.tid;
    if (tid) { return tid; }
    return null;
};
Session.prototype.model = function () {
    var model = this.runtime.profile.model;
    if (model) {
        return model;
    } else {
        return "n/a";
    }
};
Session.prototype.acct = function (no_prefix) {
    var acct = this.runtime.areas["01"].acct;
    if (acct !== null) {
        acct = acct.toString(16).toUpperCase();
        while (acct.length < 4) {
            acct = "0" + acct;
        }
        if (!no_prefix) {
            var prefix = this.daemon.prefix();
            if (prefix) {
                acct = prefix + acct;
            }
        }
        acct = acct.substr(-8);
    }
    return acct;
};

Session.prototype.send_cmd = function (cmd) {
    this.log("[error] virtual method called: Session.send_cmd", cmd);
};
Session.prototype.log = function () {
    if (!this.debug) { return; };
    var msg = "";
    for (var i = 0; i < arguments.length; i++) {
        msg = msg + arguments[i] + " ";
    }
    msg = msg.replace(/[\r]/g, "<CR>");
    msg = msg.replace(/[\n]/g, "<LF>");
    msg = "[" + this.sessionId + "][" + this.acct() + "]" + msg;
    this.daemon.log(msg);
};

module.exports = Session;
