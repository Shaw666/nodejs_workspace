'use strict';
var fs = require("fs");
/*jslint vars:true*/

require.searchCache = function (moduleName, callback) {
    // Resolve the module identified by the specified name
    var mod = require.resolve(moduleName);

    // Check if the module has been resolved and found within
    // the cache
    if (mod && ((mod = require.cache[mod]) !== undefined)) {
        // Recursively go over the results
        (function run(mod) {
            // Go over each of the module's children and
            // run over it
            mod.children.forEach(function (child) {
                run(child);
            });

            // Call the specified callback providing the
            // found module
            callback(mod);
        })(mod);
    }
};

require.uncache = function (moduleName) {
    // Run over the cache looking for the files
    // loaded by the specified module name
    require.searchCache(moduleName, function (mod) {
        console.log("delete cache", mod.id);
        delete require.cache[mod.id];
    });

    // Remove cached paths to the module.
    // Thanks to @bentael for pointing this out.
    Object.keys(module.constructor._pathCache).forEach(function (cacheKey) {
        if (cacheKey.indexOf(moduleName) > 0) {
            delete module.constructor._pathCache[cacheKey];
        }
    });
};

function API_man(web, root, route) {
    if (root.slice(-1) === "/") {
        root = root.substring(0, root.length - 1);
    }
    if (route.slice(-1) === "/") {
        route = route.substring(0, route.length - 1);
    }
    this.web         = web;
    this.web.api_man = this;
    this.root        = root;
    this.route       = route;
    this.handles     = {};
}

API_man.prototype.on_request = function (req, res, next) {
    if (this.handles[req.method]
        && this.handles[req.method][req.path]) {
        console.log("[API call]:", req.method, req.path);
        this.handles[req.method][req.path].func(req, res, next);
    } else {
        next();
    }
};
API_man.prototype.get_method = function (filename) {
    var i = filename.lastIndexOf(".");
    if (i < 0) { return null; }
    var api = filename.substring(0, i);
    var ext = filename.substring(i + 1);
    if (ext !== 'js') { return null; }
    var method = "GET";
    if (api.slice(-1) === ']') {
        i = api.lastIndexOf("[");
        if (i > 0) {
            method = api.substring(i + 1);
            method = method.substring(0, method.length - 1);
            api    = api.substring(0, i);
        }
    }
    return method;
};
API_man.prototype.load_apis = function (file, route) {
    if (fs.statSync(file).isDirectory()) {
        var fw  = fs.watch(file, this.file_monitor);
        fw.self = this;
        fw.path = file;
        fs.readdirSync(file).forEach(function (filename) {
            if (/^\.\#/.test(filename)) { return; }
            if (/.*_flymake\.js$/.test(filename)) { return; }
            this.load_apis(file + "/" + filename,
                           route + "/" + filename);
        }.bind(this));
    } else {
        var i = route.lastIndexOf(".");
        if (i < 0) { return; }
        var api = route.substring(0, i);
        var ext = route.substring(i + 1);
        if (ext !== 'js') { return; }
        var method = "GET";
        if (api.slice(-1) === ']') {
            i = api.lastIndexOf("[");
            if (i > 0) {
                method = api.substring(i + 1);
                method = method.substring(0, method.length - 1);
                api    = api.substring(0, i);
            }
        }
        var func = null;
        try {
            func = require(file);
            if (typeof func !== 'function') {
                console.log("not a valid js script: ", file);
                return;
            }
        } catch (error) {
            console.log(file, error);
            return;
        }
        var rec = {
            method : method,
            func   : func,
            route  : api,
            file   : file
        };
        console.log("[Web API load]:", api, file);
        if (!this.handles[method]) { this.handles[method] = {}; }
        this.handles[method][api] = {func : func};
    }
};

API_man.prototype.remove_route = function (method, api) {
    var routers = this.web._router.stack;
    var found  = false;
    for (var i = 0; i < routers.length; i++) {
        var route = routers[i].route;
        if (!route || !route.path || !route.method) { continue; }
        if (method === "GET") {
            if (route.path === api && route.method.get) {
                found = true;
                break;
            }
        } else if (method === "POST") {
            if (route.path === api && route.method.post) {
                found = true;
                break;
            }
        }
    }
    if (!found) { return; };
    routes.splice(i, 1);
};

API_man.prototype.on_file_changed = function (event, path, filename) {
    var file = path + "/" + filename;
    var exists;
    try {
        fs.accessSync(file, fs.F_OK);
        exists = true;
    } catch (error) {
        exists =  false;
    }
    var method, api;
    if (!exists) {
        if (!/\.js$/.test(filename)) { return; }
        if (/^\.\#/.test(filename)) { return; }
        if (/.*_flymake\.js$/.test(filename)) { return; }
        if (/\.js$/.test(file)) {
            method = this.get_method(file);
            if (!method) { return; }
            api = this.route + file.substring(this.root.length);
            api = api.substring(0, api.length - 3);
            delete this.handles[method][api];
            this.remove_route(method, api);
            require.uncache(file);
            console.log("[API DELETE]", method, api, file);
            return;
        }
    } else {
        var is_dir = false;
        try {
            is_dir = fs.statSync(file).isDirectory();
        } catch (error) {
        }
        if (is_dir) {
            console.log("[API NEW DIRECTORY]", file);
            var fw = fs.watch(file, this.file_monitor);
            fw.self = this;
            fw.path = file;
        } else {
            if (!/\.js$/.test(filename)) { return; }
            if (/^\.\#/.test(filename)) { return; }
            if (/.*_flymake\.js$/.test(filename)) { return; }
            if (/\.js$/.test(file)) {
                method = this.get_method(file);
                if (!method) { return; }
                api = this.route + file.substring(this.root.length);
                api = api.substring(0, api.length - 3);
                delete this.handles[method][api];
                this.remove_route(method, api);
                require.uncache(file);
                var func = null;
                try {
                    func = require(file);
                    if (typeof func !== 'function') { return; }
                } catch (error) {
                    console.log(file, error);
                    return;
                }
                if (!this.handles[method]) { this.handles[method] = {}; }
                this.handles[method][api] = {func : func};
                console.log("[API CHANGE or NEW]", method, api, file);
                return;
            }
        }
    }
    return;
};
API_man.prototype.file_monitor = function (event, filename) {
    API_man.prototype.on_file_changed.call(this.self, event,
                                 this.path, filename);
    
};
API_man.prototype.mount = function () {
    this.web.all("*", this.on_request.bind(this));
    this.load_apis(this.root, this.route);
};

module.exports = API_man;
