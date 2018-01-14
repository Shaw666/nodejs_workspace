'use strict';
var fs = require('fs');

var envs = ['JSYS', 'APP_HOME', 'cwcdn_conf', 'JRAM', 'JVAR', 'BOARD_DEV_LCD', 'BOARD_SERIAL0'];
var env = process.env;

var missed = [];
for (var i = 0; i < envs.length; i++) {
    if (!env[envs[i]]) { missed.push(envs[i]); };
}
if (missed.length > 0) {
    console.log("Env var missed:", missed.join(","));
    process.exit(3);
}

var config_file          = env.JSYS     + "/etc/config.json";
var config_file_default  = env.APP_HOME + "/etc/config.default.json";
var config = {
    ipr : {},
    cwcdn : {},
    modules : [],
    credit : {
	    connections : 5000,
	    lines       : 2
    },
    gateway:{}
};

process.on('uncaughtException', (err) => {
    console.log(`Caught exception: ${err}`);
    console.log(err.stack);
    process.exit(-2);
});

process.on("exit", function () {
    process.kill(process.pid);
});

function critical_error() {
    process.exit(-1);
}

function load_config() {
    var cwcdn;
    try {
        cwcdn = fs.readFileSync(env["cwcdn_conf"]).toString();
    } catch (error) {
        console.log("Missing cwcdn.conf");
        return false;
    }
    var list = cwcdn.split("\n");
    for (var i = 0; i < list.length; i++) {
        var f = list[i].split("=");
        var name = f[0];
        if (!name) { continue; };
        f.shift();
        var value = f.join("=");
        if (value.length >= 2
            && value[0] === '"'
            && value[value.length - 1] === '"') {
            value = value.substring(1, value.length - 2);
        }
        config.cwcdn[name] = value;
        
    }

    var flags = fs.readdirSync(env.JFLAGS);
    console.log("flags = ", flags);
    config.modules = [];
    for (var i = 0; i < flags.length; i++) {
	    var name = flags[i].split("_");
	    if (name.length < 3 || name[0] !== "module") { continue; };
        var model_name = name[2].toUpperCase();
	    config.modules.push(model_name);
        if (model_name === "CN0801") {
	        config.credit.lines = 8;
        }
        if (model_name === "CN0802") {
            config.gateway.tid = name[1];
        }

    }
    // if (config.modules.indexOf("CN0801") >= 0) {
	//     config.credit.lines = 8;
    // }

    try {
        var ipr    = fs.readFileSync(config_file);
        config.ipr = JSON.parse(ipr);
	return true;
    } catch (error) {
        console.log("Missing config file:", config_file);
        console.log(error);
    }

    try {
        var ipr    = fs.readFileSync(config_file_default);
        config.ipr = JSON.parse(ipr);
        fs.writeFilSync(config_file, ipr);
    } catch (error) {
        console.log("Can not restore config file to default");
        console.log(error);
    }

    return false;

};

if (!load_config()) {
    critical_error();
}

console.log("config = ", config);

fs.watch(config_file, function (event, file) {
    load_config();
});
var IPR = require("./server.js");
var ipr = new IPR(config)
ipr.config_file = config_file;
ipr.config_file_default = config_file_default;
ipr.run();

var web = require("../web/ipr-web-main.js");
web.server = ipr;
var web_port = config.ipr["web-port"]
web.listen(web_port, function () {
    console.log('web server listening on port:', web_port);
});

