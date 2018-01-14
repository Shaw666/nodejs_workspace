'use strict';
/*jslint maxlen:160, vars: true*/
var express = require('express');
var router  = express.Router();
var http    = require('http');
var fs      = require("fs");
var hanson  = require('hanson');

var partials = {
    header      : 'header',
    page_header : 'page_header',
    nav         : 'nav',
    footer      : 'footer'
};


var SESSION_ALIVE_TIME = 1000 * 60 * 30; //  3 minutes
// var SESSION_ALIVE_TIME = 1000 * 10; //  3 minutes
/*
  sessions = {
      <sid> : {
          user : <username>,
          time : <time>
      }
  }
*/
var sessions = {};
function new_sid() {
    var i;
    var sid = "";
    while ((sid === "") || sessions[sid]) {
        for (i = 0; i < 10; i++) {
            sid = Math.random() * 100000000 + '0';
        }
    }
    return sid;
}

function check_session_timeout() {
    var sid;
    for (sid in sessions) {
        if (sessions.hasOwnProperty(sid)) {
            var t = new Date() - sessions[sid].time;
            if (t > SESSION_ALIVE_TIME) {
                delete sessions[sid];
                console.log("session : ", sid, "deleted.");
            }
        }
    }
}

setInterval(check_session_timeout, 2000);
router.all("*", function (req, res, next) {
    if (req.query.key === req.app.box.server_api_key) {
        next();
    } else if (req.query.sid) {
        next();
    } else {
        console.log("cookies:", req.cookies);
        if (sessions[req.cookies.sid]) {
            next();
        } else {
            if (["/login", "/do_login"].indexOf(req.path) >= 0) {
                next();
            } else {
                res.redirect("/login");
            }
        }
    }
});
/* GET home page. */
router.get('/', function (req, res) {
    res.render('index', {
        title: 'CN8000',
        partials: partials
    });
});
/* get server status */
router.get('/get_server_status', function (req, res) {
    var url = "http://localhost:"
        + req.app.box.server_port_node_web_api
        + "/call/get_server_status";
    var tids = req.query.tids;
    if (tids) {
        url = url + "?tids=" + tids;
    }
    console.log("get : ", url);
    var q = http.request(url, function (box_res) {
        box_res.setEncoding('utf8');
        var data = "";
        box_res.on("data", function (chunk) {
            data = data + chunk;
        });
        box_res.on("end", function () {
            res.end(data);
        });
        box_res.on('error', function (e) {
            res.status(500).end();
            console.log(e);
        });
    });
    q.on('error', function (e) {
        console.log(e);
        res.status(500).end();
    });
    q.end();
});

/* download proxy */
router.get('/download/:id', function (req, res) {
    if (!req.query.sid) {
        res.status(403).end();
        return;
    }
    var url;
    var box = req.app.box;
    if (box.is_app_root || box.is_root) {
        if (box.server_fs_url) {
            url = box.server_fs_url;
        } else {
            url = "http://localhost:" + box.server_port_fs + "/download/";
        }
    } else {
        url = "http://localhost"
            + ":" + box.server_port_node_web_api
            + "/download/";
    }
    if (!url || !req.params.id) {
        res.status(404).end();
    }
    if (url[url.length - 1] !== "/") {
        url = url + "/";
    }
    url = url + req.params.id;
    url = url + "?sid=" + req.query.sid;
    console.log("get image from : ", url);
    var q = http.request(url, function (box_res) {
        box_res.on("data", function (chunk) {
            res.write(chunk);
        });
        box_res.on("end", function () {
            res.end();
        });
        box_res.on('error', function (e) {
            res.status(500).end();
            console.log(e);
        });
    });
    q.on('error', function (e) {
        console.log(e);
        res.status(500).end();
    });
    q.end();
});

/* GET settings page. */
router.get('/set-jnode', function (req, res) {
    res.render('set-jnode', {
        title: 'CN8000',
        partials: partials
    });
});

router.get('/set-relay', function (req, res) {
    res.render('set-relay', {
        title: 'CN8000',
        partials: partials
    });
});

router.get('/set-pwd', function (req, res) {
    res.render('set-pwd', {
        title: 'CN8000',
        partials: partials
    });
});

/* GET settings page. */
router.get('/set-net', function (req, res) {
    res.render('set-net', {
        title: 'CN8000',
        partials: partials
    });
});

/* GET time. */
router.get('/get_time', function (req, res) {
    var exec = require('child_process').exec, child;

    child = exec('date +"%Y-%m-%d %T" ', function (error, stdout, stderr) {
        // console.log('stdout: ' + stdout);
        // console.log('stderr: ' + stderr);
        if (error !== null) {
            console.log('exec error: ' + error);
            res.status(500).end(error);
        } else {
            res.end(stdout);
        }
    });
});

/* SET time. */
router.get('/set_time', function (req, res) {
    if (req.query.key !== req.app.box.server_api_key) {
        res.status(403).end();
    } else if (!req.app.box.server_allow_api_timesync) {
        res.status(403).end();
    } else {
        var t = req.query.time.toString();
        var exec = require('child_process').exec;
        var cmd = 'date -s "' + t + '"';
        var child = exec(cmd, function (error, stdout, stderr) {
            // console.log('stdout: ' + stdout);
            // console.log('stderr: ' + stderr);
            if (error !== null) {
                console.log('exec error: ' + cmd + " : " + error);
                res.status(500).end(error);
            } else {
                res.end("OK");
            }
        });
    }
});

/* reboot. */
router.get('/reboot', function (req, res) {
    if (req.query.key !== req.app.box.server_api_key) {
        res.status(403).end();
    } else if (!req.app.box.server_allow_api_reboot) {
        res.status(403).end();
    } else {
        var exec = require('child_process').exec;
        var cmd = 'reboot ';
        var child = exec(cmd, function (error, stdout, stderr) {
            // console.log('stdout: ' + stdout);
            // console.log('stderr: ' + stderr);
            if (error !== null) {
                console.log('exec error: ' + cmd + " : " + error);
                res.status(500).end(error);
            } else {
                res.end("OK");
            }
        });
    }
});

function res_with_external_command(req, res, command) {
    var exec = require('child_process').exec, child;

    child = exec(command, function (error, stdout, stderr) {
        // console.log('stdout: ' + stdout);
        // console.log('stderr: ' + stderr);
        if (error !== null) {
            console.log('exec error: ' + error);
        } else {
	        var result;
	        try {
		        result = JSON.parse(stdout);
	        } catch (e) {
		        result = {};
	        }
            res.end(JSON.stringify(result));
        }
    });
}
/* GET settings data. */
router.get('/jnode-get', function (req, res) {
    res_with_external_command(req, res, "jnode-get");
});


router.get('/box-net-get', function (req, res) {
    res_with_external_command(req, res, "box-net-get");
});
router.get('/box-net-stat', function (req, res) {
    res_with_external_command(req, res, "box-net-stat");
});

/*
{ 'net-route-default-gateway': '192.168.88.2',
  'net-route-default-gateway-dev': 'eth0',
  'net-eth0-active': 'true',
  'net-eth0-mode': 'static',
  'net-eth0-gateway': '192.168.88.2',
  'net-eth0-ip': '192.168.88.139',
  'net-eth0-mask': '255.255.255.0',
  'net-eth1-active': 'true',
  'net-eth1-mode': 'static',
  'net-eth1-gateway': '192.168.88.2',
  'net-eth1-ip': '192.168.1.10',
  'net-eth1-mask': '255.255.255.0',
  'net-nameserver': '8.8.8.8,114.114.114.114' }

  #eth0 is up
  auto eth0
  iface eth0 inet static
  address 192.168.88.139
  netmask 255.255.255.0
  gateway 192.168.88.2
  post-up route add default via 192.168.88.2 dev eth0


*/
router.get('/set-password', function (req, res) {
    var pass_old = req.query["old"];
    var pass_new = req.query["new"];
    console.log(pass_old, pass_new);
    var config = fs.readFileSync("/etc/box.json");
    config = JSON.parse(config);
    var result = {
        result : "OK"
    };
    if (config.admin.password !== pass_old) {
        result.result = "密码错误";
    } else {
        config.admin.password = pass_new;
        fs.writeFileSync("/etc/box.json",
                         JSON.stringify(config, null, 4));
    }
    res.end(JSON.stringify(result));
});
router.get('/box-net-set', function (req, res) {
    var net = req.query.data;
    if (!net) {
        res.status(400).end();
        return;
    }
    try {
        net = JSON.parse(net);
    } catch (error) {
        res.status(400).end();
        return;
    }
    console.log(net);
    var s = "";
    if (net["net-eth0-active"] === "true") {
        s = "auto eth0\n";
    } else {
        s = "#auto eth0\n";
    }
    s = s + "iface eth0 inet " + net["net-eth0-mode"] + "\n";
    s = s + "address " + net["net-eth0-ip"] + "\n";
    s = s + "netmask " + net["net-eth0-mask"] + "\n";
    s = s + "gateway " + net["net-eth0-gateway"] + "\n";

    if (net["net-route-default-gateway-dev"] === "eth0") {
        s = s + "post-up route add default via "
            + net["net-route-default-gateway"] + " dev eth0\n";
    }
    fs.writeFileSync("/etc/network/eth0", s);
    console.log("eth0 = \n", s);

    s = "";
    if (net["net-eth1-active"] === "true") {
        s = "auto eth1\n";
    } else {
        s = "#auto eth1\n";
    }
    s = s + "iface eth1 inet " + net["net-eth1-mode"] + "\n";
    s = s + "address " + net["net-eth1-ip"] + "\n";
    s = s + "netmask " + net["net-eth1-mask"] + "\n";
    s = s + "gateway " + net["net-eth1-gateway"] + "\n";

    if (net["net-route-default-gateway-dev"] === "eth1") {
        s = s + "post-up route add default via "
            + net["net-route-default-gateway"] + " dev eth1\n";
    }

    fs.writeFileSync("/etc/network/eth1", s);
    console.log("eth1 = \n", s);

    var dns = net["net-nameserver"].split(/[ ,;]/);
    s = "";
    for (var i = 0; i < dns.length; i++) {
        s = s + "nameserver " + dns[i] + "\n";
    }
    fs.writeFileSync("/etc/resolvconf/resolv.conf.d/base", s);

    var result = {
        result: "OK"
    };
    res.end(JSON.stringify(result));
    var exec = require('child_process').exec;
    exec("box-net-restart");
});
router.get('/jnode-set', function (req, res) {
    var config = req.query.data;
    if (!config) {
        res.status(400).end();
        return;
    }
    var config_new;
    try {
        config_new = JSON.parse(config);
    } catch (error) {
        res.status(400).end();
        return;
    }
    console.log(config_new);
    var config_old = fs.readFileSync("/etc/box.json");
    config_old = JSON.parse(config_old);
    fs.writeFileSync("/etc/box.json",
                     JSON.stringify(config_new, null, 4));
    var result = {
        result: "OK"
    };
    
    res.end(JSON.stringify(result));
    // var old_port = req.app.box.server_dashboard_web_port;
    // req.app.reload_box_json(req.app);
    // var new_port = req.app.box.server_dashboard_web_port;
    // console.log(old_port, new_port);
    // if (old_port !== new_port) {
    //     process.exit();
    // }
    // console.log("boxset exec ok");
    // res.redirect("/settings");
});

/* GET login page. */
router.get('/login', function (req, res) {
    res.render('login', {
        title: 'CN8000',
        partials: partials
    });
});

/* GET logout page. */
router.get('/logout', function (req, res) {
    res.clearCookie("sid");
    res.redirect('/login');
});

/* POST settings page. */
router.post('/do_login', function (req, res) {
    if (fs.existsSync("/etc/box.json")) {
        var str = fs.readFileSync("/etc/box.json").toString();
        try {
            var config = hanson.parse(str);
            if (req.body.username === "admin"
                && req.body.password === config.admin.password) {

                console.log("set cookies");
                var sid = new_sid();
                sessions[sid] = {
                    user : req.body.username,
                    time : new Date()
                };
                res.cookie("sid", sid,  {
                    maxAge: SESSION_ALIVE_TIME,
                    httpOnly: true
                });
                res.redirect('/');
            } else {
                res.redirect('/login');
            }
        } catch (error) {
            res.status(500).end();
        }
    } else {
        res.status(500).end();
    }
});

module.exports = router;
