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

function Session_ipr_1(server, daemon, sessionId, socket) {
    Session_tcp.call(this, server, daemon, sessionId, socket);
    this.package_buffer     = "";
    this.password           = "";
    this.idle_timer_counter = 0;
    var msg = LF + "GPS ACP" + CR;
    this.log("==>[" + msg + "]");
    this.send(msg);
}
util.inherits(Session_ipr_1, Session_tcp);

Session_ipr_1.prototype.on_package = function (data) {
    var cmds  = data.split(",");
    var cmd   = cmds.shift();
    var param = cmds;
    var ack   = CR + LF + "UFOP RCV" + CR + LF;
    this.log("==>[" + ack + "]");
    this.send(ack);
    this.on_dev_cmd(cmd, param);
};
/*

2016-05-19 17:37:24 [7:7112][8008-2653][null]<==[COWN,COWN-LYD-QT-LG9,cn1100,12,,bosch,]
2016-05-19 17:37:24 [7:7112][8008-2653][null]==>[SEND OK]
2016-05-19 17:37:24 [7:7112][8008-2653][null]<==[V,COWN-LYD-QT-LG9,,,025300290087F600501111,27,]
2016-05-19 17:37:24 [7:7112][8008-2653][null]==>[SEND OK]
2016-05-19 17:37:24 [7:7112][8008-2653][null]<==[STP,COWN-LYD-QT-LG9,223.255.9.21,7112,,,,,30,3341,]
2016-05-19 17:37:24 [7:7112][8008-2653][null]==>[SEND OK]
2016-05-19 18:07:39 [7:7112][9275-7833][70003341]<==[W,COWN-LYD-QT-LG9,,,,3341181401000017]

*/
Session_ipr_1.prototype.on_report = function (msg) {
    var area_status_xref = {
        "4" : "alarm",
        "1" : "open",
        "2" : "away",
        "3" : "stay",
        "0" : "unready",
        "6" : "awaydelay",
        "7" : "staydelay"
    };
    var type = msg[0];
    var report = "";
    msg = msg.substring(1);
    switch (type) {
    case "A": // area status
        var stat = area_status_xref[msg[0]];
        if (!stat) { stat = "unknown"; }
        report = report + "host:" + stat + ",";
        report = report + "power:" +   (msg[1] === "1" ? "trb" : "ok") + ",";
        report = report + "line:" +    (msg[2] === "1" ? "trb" : "ok") + ",";
        report = report + "battery:" + (msg[3] === "1" ? "trb" : "ok") + ",";
        report = "status," + report;
        break;
    case "B": // multi zone status
        var base = parseInt(msg[0], 10);
        for (var i = 1; i < msg.length; i++) {
            var zone = (base - 1) * 8 + i;
            var stat = "";
            if (msg[i] === '1') { stat = "u";}
            else if (msg[i] === '0') { stat = "n"; }
            else if (msg[i] === '3') { stat = "b"; }
            else if (msg[i] === '2') { stat = "a"; }
            if (stat) {
                report = report +zone + ":" + stat + ",";
            }
        }
        report = "zonestatus," + report;
        break;
    case "C": // zone status
        
        break;
    case "D": // exec result
        if (msg[7] === "0") {
        } else if (msg[7] === "1") {
            this.log("Command exec error: ", msg);
        } else if (msg[7] === "2") {
            this.log("Command not support: ", msg);
        } else {
            this.log("Unknown exec result: ", msg);
        }
        report = "";
        break;
    default:
        this.log("Unknown msg: ", msg);
        break;
    }
    if (report) {
        this.log("report: ", report);
        this.emit("stat-report", this, report);
    }
};

/*
2016-06-02 15:20:40 [server] <==[8] <LF>#700003
2016-06-02 15:20:40 [server] <==[7] 336 8 <CR>
2016-06-02 15:20:40 [server] level max =  10000
2016-06-02 15:20:40 [server] [cmd send to] 112.94.64.198 8
2016-06-02 15:20:40 [7:7112][6543-9512][3336]cmd =  8 param =
2016-06-02 15:20:40 [7:7112][6543-9512][3336]==>[<CR><LF>$KPA,,86000#<CR><LF>]
2016-06-02 15:20:40 [7:7112][6543-9512][3336]==>[<CR><LF>$KPA,,87000#<CR><LF>]
2016-06-02 15:20:40 [7:7112][6543-9512][3336]==>[<CR><LF>$KPA,,88000#<CR><LF>]
2016-06-02 15:20:41 [server] <==[8] <LF>#700003
2016-06-02 15:20:41 [server] <==[7] 336 8 <CR>
2016-06-02 15:20:41 [server] level max =  10000
2016-06-02 15:20:41 [server] [cmd send to] 112.94.64.198 8
2016-06-02 15:20:41 [7:7112][6543-9512][3336]cmd =  8 param =
2016-06-02 15:20:41 [7:7112][6543-9512][3336]==>[<CR><LF>$KPA,,86000#<CR><LF>]
2016-06-02 15:20:41 [7:7112][6543-9512][3336]==>[<CR><LF>$KPA,,87000#<CR><LF>]
2016-06-02 15:20:41 [7:7112][6543-9512][3336]==>[<CR><LF>$KPA,,88000#<CR><LF>]
2016-06-02 15:20:42 [7:7112][9771-4995][9527]<==[V,COWN-U42-9J-6Q1,,,015300290087F600501111,31,]
2016-06-02 15:20:42 [7:7112][9771-4995][9527]==>[<CR><LF>UFOP RCV<CR><LF>]
2016-06-02 15:20:43 [server] <==[8] <LF>#700003
2016-06-02 15:20:43 [server] <==[7] 336 8 <CR>
2016-06-02 15:20:43 [server] level max =  10000
2016-06-02 15:20:43 [server] [cmd send to] 112.94.64.198 8
2016-06-02 15:20:43 [7:7112][6543-9512][3336]cmd =  8 param =
2016-06-02 15:20:43 [7:7112][6543-9512][3336]==>[<CR><LF>$KPA,,86000#<CR><LF>]
2016-06-02 15:20:43 [7:7112][6543-9512][3336]==>[<CR><LF>$KPA,,87000#<CR><LF>]
2016-06-02 15:20:43 [7:7112][6543-9512][3336]==>[<CR><LF>$KPA,,88000#<CR><LF>]
[API call]: GET /api/get-server-status
2016-06-02 15:20:47 [server] ping result 0<LF>
[API call]: GET /api/get-server-status
2016-06-02 15:20:48 [server] <==[8] <LF>#700003
2016-06-02 15:20:48 [server] <==[7] 336 8 <CR>
2016-06-02 15:20:48 [server] level max =  10000
2016-06-02 15:20:48 [server] [cmd send to] 112.94.64.198 8
2016-06-02 15:20:48 [7:7112][6543-9512][3336]cmd =  8 param =
2016-06-02 15:20:48 [7:7112][6543-9512][3336]==>[<CR><LF>$KPA,,86000#<CR><LF>]
2016-06-02 15:20:48 [7:7112][6543-9512][3336]==>[<CR><LF>$KPA,,87000#<CR><LF>]
2016-06-02 15:20:48 [7:7112][6543-9512][3336]==>[<CR><LF>$KPA,,88000#<CR><LF>]
[API call]: GET /api/get-server-status

*/
Session_ipr_1.prototype.on_dev_cmd_KPA = function (param) {
    
};
Session_ipr_1.prototype.on_dev_cmd_W = function (param) {
    var sn = param[0];
    if (sn[4] === "-") {
        this.runtime.profile.tid = sn;
    } else {
        this.runtime.profile.sn = sn
    }
    if (param[4].substring(0, 6) === "FFFFFF") {
        this.on_report(param[4].substring(6));
        return;
    }
    var acct = param[4].substring(0, 4).replace(/A/g, "0");
    this.set_acct(acct);
    var cid = param[4].substring(4).replace(/A/g, "0");
    var ER     = cid.substring(2, 3);
    var ercode = cid.substring(3, 6);
    var GG     = cid.substring(6, 8);
    var CU     = "";
    var cucode = cid.substring(8, 11);
    if (ER === '1') {
        ER = "E";
    } else if (ER === '3') {
        ER = "R";
    }
    if (ercode[0] === '4') {
        CU = "U";
    } else {
        CU = "C";
    }
    var event = {};
    event.time = new Date();
    event.time_src = "server";
    event.eid = "";
    event.cid =
        this.acct()
        + " " + "18"
        + " " + ER + ercode
        + " " + GG
        + " " + CU + cucode;
    event.caller = this.sn();
    this.log("[cid event]", event.cid);
    this.emit('cid-event', this, event);
};
Session_ipr_1.prototype.on_dev_cmd_COWN = function (param) {
    // [COWN,COWN-U42-9J-6Q1,cn1100,12,,crow,]
    this.runtime.profile.tid   = param[0];
    this.runtime.profile.model = param[1];
    this.runtime.profile.ver   = param[2];
    this.runtime.profile.brand = param[4];
    var report = "prop,";
    report = report + "ver:" + this.runtime.profile.ver;
    report = report + ",type:" + this.runtime.profile.model;
    report = report + ",paneltype:" + this.runtime.profile.brand;
    var handle = setInterval(function () {
        if (this.acct()) {
            this.emit("stat-report", this, report);
            clearTimeout(handle);
        };
    }.bind(this), 5000);
};
Session_ipr_1.prototype.on_dev_cmd_STP = function (param) {
// [STP,COWN-U42-9J-6Q1,223.255.9.21,7112,,,,,30,9527,]
    this.runtime.profile.sn    = param[0];
    this.set_acct(param[8]);
    
};
Session_ipr_1.prototype.on_dev_cmd_MSG = function (param) {
    var event = {
        session : this,
        cid     : "",
        caller  : "",
        time    : null
    };
    if (param.length < 4) { return; }

    this.runtime.profile.sn = param[0];

    event.time     = param[1];
    event.time_src = "dev";

    var cid  = param[3];
    this.set_acct(cid.substring(0, 4));
    cid = cid.substring(4);
    var ER     = cid.substring(2, 3);
    var ercode = cid.substring(3, 6);
    var GG     = cid.substring(6, 8);
    var CU     = cid.substring(8, 9);
    var cucode = cid.substring(9, 12);
    event.eid = "";
    event.cid =
        this.acct()
        + " " + "18"
        + " " + ER + ercode
        + " " + GG
        + " " + CU + cucode;
    event.caller = this.sn();
    this.log("[cid event]", event.cid);
    this.emit('cid-event', this, event);
};
Session_ipr_1.prototype.on_dev_cmd = function (cmd, param) {
    this.idle_timer_counter = 0;
    if (cmd === 'MSG') {
        this.on_dev_cmd_MSG(param);
    } else if (cmd === 'COWN') {
        this.on_dev_cmd_COWN(param);
    } else if (cmd === 'W') {
        this.on_dev_cmd_W(param);
    } else if (cmd === 'STP') {
        this.on_dev_cmd_STP(param);
    } else if (cmd === 'STS') {
    } else if (cmd === 'ACT') {
    } else if (cmd === 'STN') {
    } else if (cmd === 'V') {
    } else if (cmd === 'W') {
    }
};
Session_ipr_1.prototype.do_process_buffer = function () {
    while (this.buffer.length > 0) {
        var ch =  this.buffer[0];
        this.buffer = this.buffer.substring(1);
        if (ch === '$') {
            this.package_buffer = "";
        } else if (ch === '#') {
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
Session_ipr_1.prototype.parse_ipr_cmd = function (cmd, param) {
    var cmd_xref_table = {
        "1" : "away",
        "2" : "open",
        "3" : "bypass",
        "5" : "unbypass",
        "6" : "stay",
        "8" : "get",
        "17" : "pwd"
    };
    cmd = cmd_xref_table[cmd];
    param = param.split(",");
    var result = [];
    switch (cmd) {
    case "away":
        if (!this.password) {
            this.password = param[2];
            result.push("D" + this.password);
        }
        result.push("82000");
        break;
    case "stay":
        if (!this.password) {
            this.password = param[2];
            result.push("D" + this.password);
        }
        result.push("83000");
        break;
    case "open":
        if (!this.password) {
            this.password = param[2];
            result.push("D" + this.password);
        }
        result.push("81000");
        break;
    case "bypass":
        if (!this.password) {
            this.password = param[2];
            result.push("D" + this.password);
        }
        result.push("84" + param[1].substr(-2));
        break;
    case "unbypass":
        if (!this.password) {
            this.password = param[2];
            result.push("D" + this.password);
        }
        result.push("85" + param[1].substr(-2));
        break;
    case "pwd":
        if (!this.password) {
            this.password = param[2];
            result.push("D" + this.password);
        }
        result.push("9" + param[2]);
        result.push("A" + param[0] + param[1] + '0');
        break;
    case "get":
        result.push("86000");
        result.push("87000");
        result.push("88000");
        break;
    }
    return result;
};

Session_ipr_1.prototype.send_cmd = function (cmd, param) {
    this.log("cmd = ", cmd, "param = ", param);
    var command = this.parse_ipr_cmd(cmd, param);
    for (var i = 0; i < command.length; i++) {
        var cmd = CR + LF + "$KPA," + this.tid() + ","  + command[i] + "#" + CR + LF;
        this.send(cmd);
        this.log("==>[" + cmd + "]");
    }
};

module.exports = Session_ipr_1;
