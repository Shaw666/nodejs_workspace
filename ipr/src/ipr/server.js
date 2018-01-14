
'use strict';
/*jslint vars:true */

/*
  Daemon event :

  event = {
      cid    : <CID>
      caller : <CALLER DI>
  }

  event : "cid-event"   // cid from panel with high priority.
  param : daemon, session, event

  event : "info-event"   // cid with low priority
  param : daemon, session, event
*/

var env = process.env;

var global     = require('./global.js');
var fs         = require('fs');
var exec       = require('child_process').exec, child;
var zlib       = require('zlib');

var LINE_STAT  = env.JRAM + "/line.stat";
var SYS_STAT   = env.JRAM + "/system.stat";
var SYS_LOG    = env.JVAR + "/system.log";
var EVENT_LOG  = env.JVAR + "/event.log";
var Q_TTYS0    = env.JVAR + "/ttys0.q";
var serial_dev = env.BOARD_SERIAL0;

var LATEST_EVENT_LOG  = env.JRAM   + "/latest_event.log";
var GATEWAY_RUNTIME   = env.JRAM   + "/gateway-runtime.stat";
var THINGS_CONNECTED  = env.JFLAGS + "/things-server-connected";
var DSS7016_CONNECTED = env.JFLAGS + "/dss7016-server-connected";

var LF  = String.fromCharCode(0x0A);
var CR  = String.fromCharCode(0x0D);
var SO  = String.fromCharCode(0x0E);
var DLE = String.fromCharCode(0x10);
var ACK = String.fromCharCode(0x06);
var NAK = String.fromCharCode(0x15);

// 丛文网络报警
var DRVNAME = new Buffer([0xB4, 0xD4, 0xCE, 0xC4, 0xCD, 0xF8,
                          0xC2, 0xE7, 0Xb1, 0xA8, 0xBE, 0xAF]);


function IPR_Server(config) {
    // this.debug = true;
    this.LINE_STAT  = LINE_STAT;
    this.SYS_STAT   = SYS_STAT;
    this.SYS_LOG    = SYS_LOG;
    this.EVENT_LOG  = EVENT_LOG;
    this.Q_TTYS0    = Q_TTYS0;
    this.serial_dev = serial_dev;

    this.config            = config;
    this.ttys0_state       = 'idle'; // wait-ack
    this.ttys0_buf         = '';
    this.ttys0_cmd         = '';
    this.q_ttys0_event     = [];
    this.q_ttys0_status    = [];
    this.q_ttys0_changed   = false;
    this.event_log         = [];
    this.latest_event_log  = [];
    this.event_log_changed = false;
    this.resend_timer      = null;
    this.report_seq        = 0;

    this.daemons           = [];
    this.serial_fail       = false;
    this.net_fail          = false;
    this.net_fail_cnt      = 0;
    this.sessions          = [];
    this.ignore_ack        = 0;
    this.ttys0_idle        = 20;
    this.line_status       = {};
    this.serial_in         = null;
    this.serial_out        = null;
    this.acct_index        = {};
}

IPR_Server.prototype.update_session_list = function () {
    var t1 = new Date();
    this.sessions = [];
    for (var i = 0; i < this.daemons.length; i ++) {
        var daemon = this.daemons[i];
        var list = daemon.sessions_list();
        this.sessions = this.sessions.concat(list);
        this.line_status[daemon.line()].online = list.length;
    }
    var tids = [];
    var cnt  = 100;
    var i    = 0;
    var sss  = [];
    while (i < this.sessions.length && cnt > 0) {
        var session = this.sessions[i];
        var tid   = session.runtime.profile.tid;
        var model = session.runtime.profile.model;
        if (tid && !model) {
            sss.push({
                session : session,
                tid     : tid
            })
            tids.push(tid);
            cnt--;
        }
        i++;
    }
    if (tids.length > 0) {
        var http = require('http');
        var tid = this.config.cwcdn.TID;
        var url = "http://cos.conwin.cn/tid/m?"
            + "tid=" + tid + "&tids=" + tids.join(";");
        url = url ;
        var req = http.get(url, function (res) {
            var data = "";
            res.on("data", function (trunk) {
                data = data + trunk;
            }.bind(this));
            res.on("error", function (error) {
                this.log(error);
            });
            res.on("end", function () {
                try {
                    var result = JSON.parse(data);
                    for (var i = 0; i < result.length; i++) {
                        var r = result[i];
                        for (var j = 0; j < sss.length; j++) {
                            var ss = sss[j];
                            if (ss.tid === r.tid) {
                                ss.session.runtime.profile.model = r.model;
                            }
                        }
                    }
                } catch (error) {
                }
            }.bind(this))
        }.bind(this))
        req.on("error", function (error) {
            this.log("[error] request for model:", error);
        }.bind(this))
    }
    var t2 = new Date();
    if (t2 - t1 > 100) {
        this.log("[update session list]: length=",
                 this.sessions.length, "time cost:", t2 - t1, "ms");
    }
};
IPR_Server.prototype.write_q_file = function () {
    var data;
    if (this.event_log_changed) {
        data = this.event_log.join("\n");
        if (this.event_log.length > 0) {
            data = data + "\n";
        }
        fs.writeFile(EVENT_LOG, data);

        /* 写入最后的100条事件记录到文件中 */
        data = this.latest_event_log.join("\n");
        if (this.latest_event_log.length > 0) {
            data = data + "\n";
        }
        fs.writeFile(LATEST_EVENT_LOG, data);

        this.event_log_changed = false;
    }
    if (this.q_ttys0_changed) {
        data = this.q_ttys0_event.join("\n");
        if (this.q_ttys0_event.length > 0) {
            data = data + "\n";
        }
        fs.writeFile(Q_TTYS0, data);
        this.q_ttys0_changed = false;
    }
};
IPR_Server.prototype.log_event = function (daemon, session, event) {
    var s = "";
    s = s + event.time.format() + ";";
    s = s + daemon.line() + ";";
    s = s + event.cid + ";";
    s = s + event.eid + ";";
    s = s + daemon.protocol() + ";";
    s = s + daemon.port() + ";";
    s = s + session.ip() + ";";
    s = s + session.sn() + ";";
    this.event_log.push(s);
    this.latest_event_log.push(s);
        
    if (this.event_log.length >= 100000) {
        this.event_log.shift();
    }
    if (this.latest_event_log.length >= 100) {
        this.latest_event_log.shift();
    }

    this.event_log_changed = true;
};

IPR_Server.prototype.on_info_event = function (daemon, session, event) {
    this.set_net_fail(false);
    if (["online", "offline"].indexOf(event.action) < 0) {
        return;
    }
    var report = true;
    if (!this.acct_index[event.acct]) {
        this.acct_index[event.acct] = [];
    }
    var ss = this.acct_index[event.acct];
    for (var i = 0; i < ss.length; i++) {
        if (ss[i].sessionId !== session.sessionId) {
            report = false;
            break;
        }
    }
    if (event.action === "online") {
        ss.push(session);
    } else if (event.action === "offline") {
        for (var i = 0; i < ss.length; i++) {
            if (ss[i].sessionId === session.sessionId) {
                ss.splice(i, 1);
                break;
            }
        }
    }
    if (!report) { return; };
    var format = this.config.ipr["serial-format"];
    var status = this.config.ipr["report-status-online-offline"];
    var cid = [];
    if (event.action === "online") {
        cid.push(event.acct + " 18 R3B0 00 C000");
        if ((format === "conwin") || ((format == "685") && (status == true))) {
            cid.push(event.acct + " 18 R0B0 00 C000");
        }
    } else if (event.action === "offline") {
        cid.push(event.acct + " 18 E3B2 00 C000");
        if ((format === "conwin") || ((format == "685") && (status == true))) {
            cid.push(event.acct + " 18 E0B2 00 C000");
        }
    };
    for (var i = 0; i < cid.length; i++) {
        var e = {session : session,
                 cid    : cid[i],
                 eid    : "",
                 caller : session.sn(),
                 time   : new Date()
                };
        if (this.config.ipr["log-online-offline"]) {
            this.log_event(daemon, session, e);
        }
        if (this.config.ipr["report-event-online-offline"]) {
            this.q_status_event(daemon, e);
        }
    };
};
IPR_Server.prototype.write_serial = function (msg, stx, etx) {
    this.log("==>", msg);
    if (!stx) { stx = LF; };
    if (!etx) { etx = CR; };
    this.serial_out.write(stx);
    this.serial_out.write(msg);
    this.serial_out.write(etx);
};
IPR_Server.prototype.on_stat_report = function (daemon, session, report) {
    var format = this.config.ipr["serial-format"];
    if (format !== "conwin") { return; }
    var acct = session.acct();
    while (acct.length < 8) { acct = "0" + acct; }
    var report1 = "#8" + this.report_seq + ";" + acct + " ";
    var report2 =  ";1;" + (new Buffer("report," + report)).toString('base64');
    var msg = Buffer.concat([new Buffer(report1), DRVNAME, new Buffer(report2)]);
    this.write_serial(msg, LF, SO);
    this.report_seq++;
    this.log("report: [" + report1 + DRVNAME + report2 + "]");
    
};
IPR_Server.prototype.on_cid_event = function (daemon, session, event) {
    this.set_net_fail(false);
    this.log("[cid event]", event.cid);
    this.log_event(daemon, session, event);
    var format = this.config.ipr["serial-format"];
    if (format === "685") {
        var caller = event.caller;
        if (caller) {
            this.q_ttys0_event.push(daemon.line()
                                    + "  "
                                    + session.acct().substr(-4)
                                    + caller.substring(caller.length - 10));
        }
        var arr = event.cid.split(" ");
        if (arr[0].length > 4) {
            arr[0] = arr[0].substr(-4);
        }
        event.cid = arr.join(" ");
        this.q_ttys0_event.push(daemon.line() + " " + event.cid);
    } else if (format === "conwin") {
        var s = daemon.line() + " " + event.cid;
        if (event.caller) { s = s + "#1" + event.caller + ","; }
        if (event.time) {   s = s + "#2" + event.time.format() + ","; }
        s = s + "#3" + this.surfix(daemon);
        if (event.eid) {    s = s + "#4" + event.eid + ","; }
        this.q_ttys0_event.push(s);
    } else {
        this.log("ERROR: unknown serial format:", format);
    }
    this.q_ttys0_changed = true;
    this.process_ttys0_q();
};


function uptime() {
    var t = fs.readFileSync("/proc/uptime")
        .toString()
        .split(" ")[0];
    var day     = Math.trunc(t / 86400);
    var hours   = Math.trunc((t % 86400) / 3600);
    var minutes = Math.trunc((t % 3600) / 60);
    var seconds = Math.trunc(t % 60);
    var result  = "";
    if (day) { result = result + day + " 天 "; }
    if (hours) { result = result + hours + " 小时 "; }
    if (minutes) { result = result + minutes + " 分 "; }
    if (seconds) { result = result + seconds + " 秒"; }
    return result;
}

IPR_Server.prototype.get_gateway_runtime = function (callback) {
    if (typeof callback !== 'function') { return; }
    fs.readFile(GATEWAY_RUNTIME, function (err, data) {
        if (err) {
            callback(null);
            return;
        }
        try {
            var r = JSON.parse(data);
            callback(r["online-devices"]);
        } catch (err) {
            callback(null);
        }
    });
}
    
IPR_Server.prototype.get_runtime = function (index, callback) {
    if (typeof callback !== 'function') { return; }
    var r = {};
    r["sys-uptime"]  = uptime();
    r["sys-time"]    = (new Date()).format();
    r["sys-model"]   = this.config.cwcdn.MODEL.toUpperCase();
    r["sys-sn"]      = this.config.cwcdn.TID;
    r["sys-version"] = this.config.cwcdn.VERSION;
    r["sys-os"]      = "linux "
        + fs.readFileSync("/proc/version")
        .toString()
        .split(" ")[2];
    r["sys-serial-fail"]  = this.serial_fail;
    r["sys-network-fail"] = this.net_fail;
    r["total-online"]	  = this.sessions.length;
    r["sys-modules"]	  = this.config.modules.join(", ");
    r["sys-lines"]        = this.config.credit.lines;
    r["sys-connections"]  = this.config.credit.connections;
    r["pending-event"]    = this.q_ttys0_event.length;
    r["pending-status"]   = this.q_ttys0_status.length;
    r["event-log-length"] = this.event_log.length;
    r["gateway-sn"]       = this.config.gateway.tid;
    r["things-server"]    = fs.existsSync(THINGS_CONNECTED);
    r["dss7016-server"]   = fs.existsSync(DSS7016_CONNECTED);
    r["video-online"] = 0;
    try {
	    var s = fs.readFileSync(GATEWAY_RUNTIME);
	    s = JSON.parse(s);
	    r["video-online"] = s["online-number"];
    } catch (err) {
    }
    r["sessions"]         = [];
    // var end = index + 100;
    var end = this.sessions.length;
    if (end > this.sessions.length) { end = this.sessions.length; }
    for (var i = index; i < end; i++) {
        var ss = this.sessions[i];
        r["sessions"].push({
            order    : i,
            line     : ss.daemon.line(),
            protocol : ss.daemon.protocol(),
            port     : ss.daemon.port(),
            acct     : ss.acct(),
            model    : ss.model(),
            sn       : ss.sn(),
            ip       : ss.ip(),
            uptime   : ss.uptime.format()
        });
    }

    callback(r);
};
IPR_Server.prototype.surfix = function (daemon) {
    return this.config.cwcdn.MODEL + "-" + this.config.ipr["reciver-number"];
};
IPR_Server.prototype.q_status_event = function (daemon, event) {
    var format  = this.config.ipr["serial-format"];
    var session = event.session;
    var caller  = event.caller;
    if (format === "685") {
        var cid = event.cid.split(" ");
        if (cid[0].length > 4) {
            cid[0] = cid[0].substr(-4);
        }
        cid = cid.join(" ");
        if (this.config.ipr["panel-event-first"]) {
            this.q_ttys0_status.push(daemon.line()
                                    + "  "
                                    + session.acct().substr(-4)
                                     + caller.substring(caller.length - 10));
            this.q_ttys0_status.push(daemon.line() + " " + cid);
        } else {
            this.q_ttys0_event.push(daemon.line()
                                    + "  "
                                    + session.acct().substr(-4)
                                    + caller.substring(caller.length - 10));
            this.q_ttys0_event.push(daemon.line() + " " + cid);
        }
    } else if (format === "conwin") {
        var s = daemon.line() + " " + event.cid;
        if (event.caller) { s = s + "#1" + event.caller + ","; }
        if (event.time) {   s = s + "#2" + event.time.format() + ","; }
        s = s + "#3" + this.surfix(daemon);
        if (event.eid) {    s = s + "#4" + event.eid + ","; }
        if (this.config.ipr["panel-event-first"]) {
            this.q_ttys0_status.push(s);
        } else {
            this.q_ttys0_event.push(s);
        }
    } else {
        this.log("ERROR: unknown serial format:", format);
    }
    this.process_ttys0_q();
};
IPR_Server.prototype.send_all_connection_status = function () {
    for (var i = 0; i < this.daemons.length; i++) {
        var daemon = this.daemons[i];
        var list = daemon.get_all_session_status();
        for (var j = 0; j < list.length; j++) {
            this.q_status_event(daemon, list[j]);
        }
    }
};
IPR_Server.prototype.zip_msg = function (q) {
    var num = 500;
    if (q.length < num) { num = q.length; };
    this.log("ziped", num);
    var msg = "";
    for (var i = 0; i < num; i++) {
        msg = msg + LF + this.config.ipr["reciver-number"] + q[i] + CR;
    }
    msg = DLE + zlib.deflateSync(new Buffer(msg)).toString("base64");
    return {
        msg : msg,
        num : num
    }
};
IPR_Server.prototype.get_next_message = function () {
    var msg    = null;
    var ziped  = false;
    var format = this.config.ipr["serial-format"];
    if (format !== "685" && this.q_ttys0_event.length > 10) {
        var ret = this.zip_msg(this.q_ttys0_event);
	    ziped = true;
        msg = ret.msg;
        this.pending_event = ret.num;
    } else if (this.q_ttys0_event.length > 0) {
        msg = this.q_ttys0_event[0];
        this.pending_event = 1;
    } else if (format !== "685"
               && this.q_ttys0_status.length > 10) {
        var ret = this.zip_msg(this.q_ttys0_status);
	    ziped = true;
        msg = ret.msg;
        this.pending_event = ret.num;
    } else if (this.q_ttys0_status.length > 0) {
        msg = this.q_ttys0_status[0];
        this.pending_event = 1;
    }
    if (msg && !ziped) {
        msg = this.config.ipr["reciver-number"] + msg;
    }
    return msg;
}
IPR_Server.prototype.process_ttys0_q = function () {
    if (this.ttys0_state !== 'idle') { return; }
    var msg = this.get_next_message();
    if (!msg) { return; };
    this.log("send out: ", msg);
    this.write_serial(msg);
    this.ttys0_state = "wait-ack";
    this.resend_timer =
        setTimeout(function () {
            this.ttys0_state = "idle";
            this.process_ttys0_q();
        }.bind(this), 10000);
};

IPR_Server.prototype.get_sessions_of = function (acct) {
    var result = [];
    for (var i = 0; i <  this.daemons.length; i++) {
        var daemon = this.daemons[i];
        result = result.concat(daemon.get_sessions_of(acct));
    }
    return result;
};
IPR_Server.prototype.on_ttys0_cmd = function (command) {
    if (command.substring(0, 2) !== '#7') { return; }
    if (!this.config.ipr["allow-control-device"]) { return; };
    command = command.substring(2).split(' ');
    var acct  = parseInt(command.shift(), 16);
    var cmd   = command.shift();
    var param = command.join(" ");
    var sessions = this.get_sessions_of(acct);
    if (sessions.length > 0) {
        var level_max = 10000;
        for (var i = 0; i < sessions.length; i++) {
            if (sessions[i].runtime.level < level_max) {
                level_max = sessions[i].runtime.level;
            }
        }
        this.log("level max = ", level_max);
        var level = sessions[0].level();
        for (var i = 0; i < sessions.length; i++) {
            var session = sessions[i];
            if (session.level() === level_max) {
                this.log("[cmd send to]",
                            session.runtime.tcp.raddr, cmd);
                session.send_cmd(cmd, param);
            }
        }
    }

};
IPR_Server.prototype.on_ttys0_data = function (data) {
    this.log("<==[" + data.length + "]", data);
    this.ttys0_buf = this.ttys0_buf + data.toString();
    while (this.ttys0_buf.length > 0) {
        var ch = this.ttys0_buf[0];
        this.ttys0_buf = this.ttys0_buf.substring(1);
        if (ch === ACK) {
            this.ttys0_idle = 0;
            if (this.ignore_ack > 0) {
                this.ignore_ack--;
            } else {
                if (this.resend_timer) {
                    clearTimeout(this.resend_timer);
                    this.resend_timer = null;
                }
                if (this.ttys0_state === 'wait-ack') {
                    var arr = null;
                    if (this.q_ttys0_event.length > 0) {
                        arr = this.q_ttys0_event;
                    } else if (this.q_ttys0_status.length > 0) {
                        arr = this.q_ttys0_status;
                    }
                    if (arr) {
                        arr.splice(0, this.pending_event);
                        this.q_ttys0_changed = true;
                    }
                    this.ttys0_state = 'idle';
                    this.process_ttys0_q();
                };
            }
        } else if (ch === NAK) {
            this.ttys0_idle = 0;
            if (this.resend_timer) {
                clearTimeout(this.resend_timer);
                this.resend_timer = null;
            }
            if (this.ttys0_state === 'wait-ack') {
                this.ttys0_state = 'idle';
                this.process_ttys0_q();
            }
        } else if (ch === 'S') {
            this.ttys0_idle = 0;
            this.write_serial("00 OKAY @");
            this.ignore_ack++;
        } else if (ch === 'G') {
            this.ttys0_idle = 0;
            this.send_all_connection_status();
        } else if (ch === LF) {
            this.ttys0_idle = 0;
            this.ttys0_cmd = "";
        } else if (ch === CR) {
            this.ttys0_idle = 0;
            this.on_ttys0_cmd(this.ttys0_cmd);
            this.ttys0_cmd = '';
        } else {
            this.ttys0_cmd = this.ttys0_cmd + ch;
        }
    }
};
IPR_Server.prototype.write_system_status = function () {
    var r = {};
    r["sys-uptime"]  = uptime();
    r["sys-time"]    = (new Date()).format();
    r["sys-model"]   = this.config.cwcdn.MODEL;
    r["sys-sn"]      = this.config.cwcdn.TID;
    r["sys-version"] = this.config.cwcdn.VERSION;
    r["sys-os"]      = "linux "
        + fs.readFileSync("/proc/version")
        .toString()
        .split(" ")[2];
    r["sys-serial-fail"]  = this.serial_fail;
    r["sys-network-fail"] = this.net_fail;
    r["total-online"]     = this.sessions.length;
    r["pending-event"]    = this.q_ttys0_event.length;
    r["pending-status"]   = this.q_ttys0_status.length;
    r["event-log-length"] = this.event_log.length;
    var stat = "";
    for (var key in r) {
        if (!r.hasOwnProperty(key)) { continue; };
        var value = "";
        if (typeof r[key] !== "undefined" ) { value = r[key].toString(); };
        stat = stat
            + key.toUpperCase().replace(/-/g, "_")
            + "=\"" + value+ "\"\n";
    }
    fs.writeFileSync(SYS_STAT, stat);

    stat = "";
    for (var line_key in this.line_status) {
        if (!this.line_status.hasOwnProperty(line_key)) { continue; };
        var line = this.line_status[line_key];
        for (var key in line) {
            if (!line.hasOwnProperty(key)) { continue; };
            stat = stat
                + "LINE_" + line_key + "_" + key.toUpperCase()
                + "=\"" + line[key].toString() + "\"\n";
        }
    }

    fs.writeFileSync(LINE_STAT, stat);
};

IPR_Server.prototype.load_data = function () {
    try {
        fs.accessSync(EVENT_LOG, fs.F_OK);
        var data = fs.readFileSync(EVENT_LOG).toString();
        if (data.length > 0) {
            data = data.substring(0, data.length - 1);
            this.event_log = data.split("\n");
        }
        
        /* 最后100条事件记录 */
        data = data.split("\n");
        var start_line = data.length - 100;
        for (var i = 0; i < 100; i ++) {
            this.latest_event_log.push(data[start_line + i]);
        }
    } catch(error) {
    }
    
    try {
        fs.accessSync(Q_TTYS0, fs.F_OK);
        var data = fs.readFileSync(Q_TTYS0).toString();
        if (data.length > 0) {
            data = data.substring(0, data.length - 1);
            this.q_ttys0_event = data.split("\n");
        }
    } catch(error) {
    }
    // this.log(EVENT_LOG, this.event_log);
    // this.log(Q_TTYS0, this.q_ttys0_event);
};


IPR_Server.prototype.create_daemon_ipr_1 = function (key) {
    var line = this.config.ipr["tcp-lines"][key];
    if (line.protocol !== 'ipr-1') { return; };
    if (!line.enabled) { return; };
    var Daemon = require("./daemon-ipr-1.js");
    var daemon = new Daemon(this, key);
    return daemon;
};

IPR_Server.prototype.create_daemon_ipr_2 = function (key) {
    var line = this.config.ipr["tcp-lines"][key];
    if (line.protocol !== 'ipr-2') { return; };
    if (!line.enabled) { return; };
    var Daemon = require("./daemon-ipr-2.js");
    var daemon = new Daemon(this, key);
    return daemon;
};

IPR_Server.prototype.create_daemon_phone_line = function (line) {
};

IPR_Server.prototype.start_daemon = function () {
    var lines = this.config.ipr["tcp-lines"];
    if (!lines) {
        this.log("No lines config for server");
        return;
    };
    var cnt = 0;
    for (var key in lines) {
        if (!lines.hasOwnProperty(key)) { continue; };
        var line = lines[key];
	cnt++;
	if (cnt > this.config.credit.lines) { continue; };
        if (!line.enabled) { continue; };
        this.line_status[key] = {
            key      : key,
            enabled  : line.enabled,
            name     : line.name,
            port     : line.port,
            protocol : line.protocol,
            prefix   : line.prefix,
            online   : 0
        };
        var daemon = null;
        switch (line.protocol) {
        case "ipr-1" :
            daemon = this.create_daemon_ipr_1(key);
            break;
        case "ipr-2" :
            daemon = this.create_daemon_ipr_2(key);
            break;
        }
        if (daemon) {
            this.daemons.push(daemon);
            daemon.on('cid-event',    this.on_cid_event.bind(this));
            daemon.on('stat-report',  this.on_stat_report.bind(this));
            daemon.on('info-event',   this.on_info_event.bind(this));
            daemon.start();
        }
    }
}
IPR_Server.prototype.setup_serial = function () {
    var baud = this.config.ipr["serial-baud"];
    var cmd = "stty -F " + serial_dev + " " + baud;
    this.log(cmd);
    var child = exec(cmd, function (error, stdout, stderr) {
        // console.log('stdout: ' + stdout);
        // console.log('stderr: ' + stderr);
        if (error !== null) {
            this.log('set baud error: ' + cmd + " : " + error);
        } else {
            this.log(serial_dev + ' baud set to: ' + baud);
            var f = fs.createReadStream(serial_dev);
            f.on("data", this.on_ttys0_data.bind(this));
            this.serial_in = f;
            setInterval(this.write_q_file.bind(this), 60 * 60);
            this.serial_out = fs.createWriteStream(serial_dev);
            if (this.q_ttys0_event.length > 0) {
                this.process_ttys0_q();
            }
        }
    }.bind(this));
    setInterval(this.serial_monitor.bind(this), 1000);
};
IPR_Server.prototype.serial_monitor = function () {
    this.ttys0_idle++;
    if (this.ttys0_idle > 60) {
        if (!this.serial_fail) {
            this.ttys0_cmd = "";
            this.log(0, "system", "", "", "Serial fail.");
            this.write_system_status();
        }
        this.serial_fail = true;
    } else {
        if (this.serial_fail) {
            this.log(0, "system", "", "", "Serial restore");
            this.write_system_status();
        }
        this.serial_fail = false;
    }
};
IPR_Server.prototype.set_net_fail = function (fail) {
    if (!fail) {
        if (this.net_fail) {
            this.log(0, "system", "", "", "Network restore");
            this.write_system_status();
        }
        this.net_fail = false;
    } else {
        if (!this.net_fail) {
            this.log(0, "system", "", "", "Network fail.");
            this.write_system_status();
        }
        this.net_fail = true;
    }
};
IPR_Server.prototype.network_monitor = function () {
    var cmd = "ping -c 1 $(route -n | awk '/^0\.0\.0\.0/{print $2}') > /dev/null; echo $?";
    var child = exec(cmd, function (error, stdout, stderr) {
        // console.log('stdout: ' + stdout);
        // console.log('stderr: ' + stderr);
        this.log("ping result", stdout);
	if (stdout[0] === "0") {
	    this.net_fail_cnt = 0;
	} else {
	    this.net_fail_cnt++;
	    setTimeout(this.network_monitor.bind(this), 2000);
	}
	if (this.net_fail_cnt > 15) {
	    this.set_net_fail(true);
	} else {
	    this.set_net_fail(false);
	}
    }.bind(this))
};
IPR_Server.prototype.set_beep_monitor = function () {
    if (this.config.ipr["monitor-beep-serial"]) {
	    fs.openSync(env.JFLAGS + "/monitor-beep-serial", "w");
    }
    if (this.config.ipr["monitor-beep-net"]) {
	    fs.openSync(env.JFLAGS + "/monitor-beep-net", "w");
    }
};
IPR_Server.prototype.run = function () {
    this.log(0, "system", "", "", "系统服务启动");
    this.start_daemon();
    this.setup_serial();
    this.load_data();
    this.set_beep_monitor();
    setInterval(this.network_monitor.bind(this), 10000);
    setInterval(this.update_session_list.bind(this), 10000);
    setInterval(this.write_system_status.bind(this), 5000);
}

var ascii = ["NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL", "BS",
 "HT", "LF", "VT", "FF", "CR", "SO", "SI", "DLE", "DC1", "DC2",
 "DC3", "DC4", "NAK", "SYN", "ETB", "CAN", "EM", "SUB", "ESC",
 "FS", "GS", "RS", "US"];

IPR_Server.prototype.log = function () {
    var level = -1;
    var first = 0;
    var user, ip, port;
    if (typeof arguments[0] === 'number') {
        level = arguments[0];
        user  = arguments[1] || "";
        ip    = arguments[2] || "";
        port  = arguments[3] || "";
        first = 4;
    }
    if (level !== 0 && !this.debug) { return; };
    var msg = "";
    for (var i = first; i < arguments.length; i++) {
        msg = msg + arguments[i] + " ";
    }
    if (level === 0) {
        msg = (new Date()).format() + ";"
            + user + ";"
            + ip + ";"
            + port + ";"
            + msg;
        fs.appendFileSync(SYS_LOG, msg + "\n");
    }
    var result = "";
    for (var i = 0; i < msg.length; i++) {
        if (msg[i] < ' ') {
            result = result + "<" + ascii[msg.charCodeAt(i)] + ">";
        } else {
            result = result + msg[i];
        }
    };
    msg = result;
    msg = msg.replace(/[^ -~]/g, "?");
    console.log((new Date()).format(), "[server]", msg);
};

module.exports = IPR_Server;

