'use strict';
/*jslint vars:true, sub:true, node:true, nomen: true, maxlen: 160, plusplus:true, stupid:true*/
var util = require('util');
var EventEmitter = require('events').EventEmitter;
var Session_tcp = require('./session-tcp.js');

var LF  = String.fromCharCode(0x0A);
var CR  = String.fromCharCode(0x0D);
var ACK = String.fromCharCode(0x06);
var NAK = String.fromCharCode(0x15);
var IDLE_TIME = 300;

function conv1(value) {
    if (value === "ok") {
        return false;
    } else if (value === "trb") {
        return true;
    }
    return null;
}
function str2pairs(str, deli1, deli2) {
    if (!deli1) { deli1 = ','; }
    if (!deli2) { deli2 = ':'; }
    var pairs = {};
    while (str.length > 0) {
        var i = str.indexOf(deli1);
        var field = str;
        if (i > 0) {
            field = str.substring(0, i);
            str = str.substring(i + 1);
        } else {
            str = "";
        }
        var name  = field;
        var value = null;
        i = field.indexOf(deli2);
        if (i > 0) {
            name  = field.substring(0, i);
            value = field.substring(i + 1);
        }
        pairs[name] = value;
    }
    return pairs;
}

function Session_ipr_2(server, daemon, sessionId, socket) {
    Session_tcp.call(this, server, daemon, sessionId, socket);
    this.package_buffer     = "";
    this.package_counter    = 0;
    this.idle_timer_counter = 0;
}
util.inherits(Session_ipr_2, Session_tcp);

Session_ipr_2.prototype.on_package = function (data) {
    var i = data.indexOf(",");
    if (i < 0) { return; }
    var pkg = {
        type : null,
        id   : null
    };
    pkg.type = data.substring(0, i);
    data = data.substring(i + 1);

    i = data.indexOf(",");
    if (i < 0) { return; }
    pkg.id = data.substring(0, i);
    data = data.substring(i + 1);

    if (pkg.type === "ack") {
        while (data.length > 0) {
            i = data.indexOf(",");
            var field = data;
            if (i > 0) {
                field = data.substring(0, i);
                data = data.substring(i + 1);
            } else {
                data = "";
            }
            var fields = field.split(":");
            if (fields.length >= 2) {
                pkg[fields[0]] = fields[1];
            }
        }
    } else if (pkg.type === "d") {
        var cmd = data;
        var param = null;
        i = data.indexOf(",");
        if (i > 0) {
            cmd = data.substring(0, i);
            param = data.substring(i + 1);
        }
        this.on_dev_cmd(cmd, param);
        cmd = LF + "ack," + pkg.id + CR;
        this.log("<==[" + cmd + "]");
        this.send(cmd);
    }
};
Session_ipr_2.prototype.on_dev_cmd_event = function (param) {
    var event = {
        session : this,
        cid     : "",
        caller  : "",
        time    : null
    };
    var pairs = str2pairs(param);
    if (!pairs.id || !pairs.cid) { return null; }
    this.set_acct(pairs.id);
    var s = pairs.cid;
    var ER     = s.substring(2, 3);
    var ercode = s.substring(3, 6);
    var GG     = s.substring(6, 8);
    var CU     = s.substring(8, 9);
    var cucode = s.substring(9, 12);
    if (pairs.time) {
        event.time = new Date(pairs.time);
        event.time_src = "dev";
    } else {
        event.time = new Date();
        event.time_src = "server";
    }
    event.eid = "";
    event.cid =
        this.acct()
        + " " + "18"
        + " " + ER + ercode
        + " " + GG
        + " " + CU + cucode;
    event.caller = this.tid();
    this.log("[cid event]", event.cid);
    this.emit('cid-event', this, event);
};
Session_ipr_2.prototype.on_dev_cmd_report = function (param) {
    var pairs = str2pairs(param);
    if (!pairs.id) { return; }
    this.set_acct(pairs.id);
    if (pairs.status === null) {
        if (pairs.host) {
            this.runtime.areas["01"].stat = pairs.host;
        }
        if (pairs.power) {
            this.runtime.flags["ac-fail"] = conv1(pairs.power);
        }
        if (pairs.battery) {
            this.runtime.flags["low-battery"] = conv1(pairs.battery);
        }
        if (pairs.line) {
            this.runtime.flags["line-fail"] = conv1(pairs.line);
        }
        if (pairs.gprs) { this.runtime.flags.gprs = pairs.gprs; }
    } else if (pairs.zonestatus === null) {
        delete pairs.id;
        delete pairs.zonestatus;
        for (var num in pairs) {
            var zone = num.toString(16);
            while (zone.length < 3) { zone = "0" + zone; }
            if (pairs.hasOwnProperty(num)) {
                this.runtime.zones[zone] = pairs[num];
            }
        }
    } else if (pairs.prop === null) {
        if (pairs.ver) { this.runtime.profile.ver = pairs.ver; }
        if (pairs.type) { this.runtime.profile.model = pairs.type; }
        // if (pairs.paneltype) {
        // this.runtime.paneltype = pairs.paneltype; }
    } else if (pairs.prg === null) {
        // delete pairs.prg;
        // for (var v in pairs) {
        //     if (pairs.hasOwnProperty(v)) {
        //         this.runtime.program[v] = pairs[v];
        //     }
        // }
    }
    var i = param.indexOf(",");
    var report = param.substring(i + 1);
    this.emit("stat-report", this, report);
};
Session_ipr_2.prototype.on_dev_cmd_hi = function (param) {
    var pairs = str2pairs(param);
    if (!pairs.id) { return; }
    if (pairs.sn) {     this.runtime.profile.sn  = pairs.sn; }
    if (pairs.sn) {     this.runtime.profile.tid = pairs.sn; }
    if (pairs.status) { this.runtime.areas["01"].stat = pairs.status; }
    if (pairs.time == '0') {
        this.send_cmd("time", new Date().format());
    }
    if (pairs.loc) { 
        var loc = pairs.loc.split(";");
        this.runtime.geo.lon    = loc[0];
        this.runtime.geo.lat    = loc[1];
    }
    if (pairs.ver) {    this.runtime.profile.ver     = pairs.ver; }
    if (pairs.type) {   this.runtime.profile.model   = pairs.type; }
    if (pairs.level) {  this.runtime.level = parseInt(pairs.level, 10); }
    this.set_acct(pairs.id);
};
Session_ipr_2.prototype.on_dev_cmd = function (cmd, param) {
    this.idle_timer_counter = 0;
    if (cmd === 'event') {
        this.on_dev_cmd_event(param);
    } else if (cmd === 'report') {
        this.on_dev_cmd_report(param);
    } else if (cmd === 'hi') {
        this.on_dev_cmd_hi(param);
    }
};
Session_ipr_2.prototype.do_process_buffer = function () {
    while (this.buffer.length > 0) {
        var ch =  this.buffer[0];
        this.buffer = this.buffer.substring(1);
        if (ch === '\n' || ch === '\r') {
            if (this.package_buffer.length > 0) {
                this.log("<==[" + this.package_buffer + "]");
                this.on_package(this.package_buffer);
                this.package_buffer = "";
            }
        } else {
            this.package_buffer = this.package_buffer + ch;
        }
    }
};
Session_ipr_2.prototype.parse_ipr_cmd = function (cmd, param) {
    var cmd_xref_table = {
        "1"   : "away",
        "2"   : "open",
        "3"   : "bypass",
        "5"   : "unbypass",
        "6"   : "stay",
        "8"   : "get",
        "17"  : "pwd",
        "18"  : "output",
        "100" : "get",
        "time": "time",
    };
    cmd = cmd_xref_table[cmd];
    param = param.split(",");
    var result = "";
    switch (cmd) {
    case "away":
    case "stay":
    case "open":
        result = cmd;
        result = result + "," + param[2];
        result = result + (param[0] ?  ("," + param[0]) : '');
        break;
    case "bypass":
    case "unbypass":
        result = cmd;
        result = result + "," + param[2];
        result = result + "," + param[1];
        break;
    case "pwd":
        result = cmd;
        result = result + "," + param[0];
        result = result + "," + param[3];
        result = result + "," + param[2];
        result = result + ":" + param[1];
        break;
    case "get":
        result = cmd + ",status,all";
        break;
    case "time":
        result = cmd + "," + param[0];
        break;
    case "output":
        result = param[0] === "1"? "output-action":"output-reset";
        result = result + "," + param[1];
        result = result + "," + param[2];
        break;
    }
    return result;
};
Session_ipr_2.prototype.send_cmd = function (cmd, param) {
    var command = this.parse_ipr_cmd(cmd, param);
    if (!command) { return; };
    this.log(command);
    var acct = this.acct(true);
    if (acct) {
        var cmd = LF + 'd,' + this.package_counter + ',ctrl,id:'
            + acct.toString(16) + "," + command + "\n";
        this.send(cmd);
        this.log("==>[" + cmd + "]");
        this.package_counter = (this.package_counter + 1) % 10000;
    }
};

module.exports = Session_ipr_2;
