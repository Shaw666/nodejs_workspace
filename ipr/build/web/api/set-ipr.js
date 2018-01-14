"use strict";var fs=require("fs");function valid_boolean(value){if(typeof value==="boolean"){return true}if(value==="true"){return true}if(value==="false"){return true}return false}var valid_field={};valid_field["reciver-number"]=function(value){if(typeof value!=="number"){return false}if(value<0||value>9){return false}return true};valid_field["serial-format"]=function(value){if(["conwin","685"].indexOf(value)<0){return false}return true};valid_field["serial-baud"]=function(value){if([9600,19200,38400,57600,115200].indexOf(value)<0){return false}return true};valid_field["panel-event-first"]=valid_boolean;valid_field["report-online-offline"]=valid_boolean;valid_field["log-online-offline"]=valid_boolean;valid_field["allow-control-device"]=valid_boolean;valid_field["confirm-offline-time"]=function(value){if(typeof value!=="number"){return false}if(value<0||value>600){return false}return true};valid_field["api-key"]=function(value){if(typeof value!=="string"){return false}if(value.length>20){return false}return true};valid_field["api-allow-set-time"]=valid_boolean;valid_field["api-allow-reboot"]=valid_boolean;valid_field["web-port"]=function(value){if(typeof value!=="number"){return false}if(value<2e3||value>1e4){return false}return true};function set_system_time(time,req){var exec=require("child_process").exec,child;var cmd='date -s "'+time+'"';child=exec(cmd,function(error,stdout,stderr){if(error!==null){req.app.server.log(0,"admin",req.socket.remoteAddress,req.socket.remotePort,"重置系统时间失败:",time,stderr);console.log("exec error: "+error)}else{req.app.server.log(0,"admin",req.socket.remoteAddress,req.socket.remotePort,"重置系统时间成功:"+time)}})}var fields_need_reboot=["web-port","serial-baud"];module.exports=function(req,res){var config=req.query.data;if(!config){res.status(400).end();return}var config_new;try{config_new=JSON.parse(config)}catch(error){res.status(400).end();return}console.log(config_new);var config_old=fs.readFileSync("/etc/ipr/config.json");config_old=JSON.parse(config_old);var fail=false;var reason="";var need_reboot=false;var new_time=config_new["system-time"];if(new_time){set_system_time(new_time,req)}for(var key in config_new){if(!config_new.hasOwnProperty(key)){continue}if(config_new[key]===config_old[key]){continue}if(key==="system-time"){continue}if(fields_need_reboot.indexOf(key)>=0){need_reboot=true}var value=config_new[key];var func=valid_field[key];if(func){if(func(value)){if(typeof config_old[key]==="boolean"){if(value==="true"){value=true}if(value==="false"){value=false}}config_old[key]=value}else{console.log("[/api/set-ipr.js] error: ",key,value);fail=true;reason=key;break}}}if(fail){res.status(400).end(reason);return}fs.writeFileSync("/etc/ipr/config.json",JSON.stringify(config_old,null,4));var result={result:"OK"};res.end(JSON.stringify(result));req.app.server.log(0,"admin",req.socket.remoteAddress,req.socket.remotePort,"接警机参数修改");if(need_reboot){process.exit(1)}};