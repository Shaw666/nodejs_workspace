'use strict';
/*jslint vars:true*/
var fs = require('fs');
/*
  [API call]: GET /api/set-net
  { 'net-route-default-gateway': '',
  'net-route-default-gateway-dev': null,
  'net-active': 'true',
  'net-mode': 'dhcp',
  'net-gateway': '',
  'net-ip': '',
  'net-mask': '',
  'net-nameserver': '' }
*/

function valid_ip(ip) {
    if (!ip) { return false; }
    if (typeof ip !== 'string') { return false; }
    ip = ip.split(".");
    if (ip.length !== 4) { return false; }
    for (var i = 0; i < 4; i++) {
        try {
            var n = parseInt(ip[i], 10);
            if (n < 0 || n > 255) { return false; }
        } catch (error) {
            return false;
        }
    }
    return true;
}
function set_nic(config) {
    var nic = "eth0";
    var mode    = config["net-mode"];
    var gateway = config["net-gateway"];
    var ip      = config["net-ip"];
    var mask    = config["net-mask"];
    console.log("[%s][%s][%s][%s]", mode, gateway, ip, mask);
    if (mode !== "dhcp" && mode !== "static") { return; }
    var text = "auto " + nic + "\n";
    if (mode === "dhcp") {
        text = text + "iface " + nic + " inet dhcp\n";
    } else {
        if (!valid_ip(ip)) { return; }
        if (!valid_ip(mask)) { return; }
        if (!valid_ip(gateway)) { return; }
        text = text + "iface " + nic + " inet static\n";
        text = text + "address " + ip + "\n";
        text = text + "netmask " + mask + "\n";
        text = text + "gateway " + gateway + "\n";
    }
    var file = "/etc/network/" + nic;
    fs.writeFileSync(file, text);
}

function set_nameserver(config) {
    var ns = config["net-nameserver"];
    if (!ns) { return; };
    ns = ns.split(/ ;,/);
    var text = "";
    for (var i = 0; i < ns.length; i++) {
        if (ns[i]) {
            text = text + "nameserver " + ns[i] + "\n";
        }
    }
    if (text) {
        var file = "/etc/resolvconf/resolv.conf.d/base";
        fs.writeFileSync(file, text);
    }
}
module.exports = function (req, res) {
    var config = req.query.data;
    if (!config) {
        res.status(400).end();
        return;
    }
    try {
        config = JSON.parse(config);
    } catch (error) {
        res.status(400).end();
        return;
    }
    console.log(JSON.stringify(config));
    res.end("{\"result\":\"OK\"}");
    set_nic(config);
    set_nameserver(config);
    var exec = require('child_process').exec;
    var command = "ifdown -a && ifup -a";
    var child = exec(command, function (error, stdout, stderr) {
        // console.log('stdout: ' + stdout);
        // console.log('stderr: ' + stderr);
        if (error !== null) {
            console.log('exec error: ' + error);
            res.status(500).end();
        } else {
	        var result;
	        try {
	        } catch (e) {
                console.log(e);
	        }
        }
    });
    return;
};
