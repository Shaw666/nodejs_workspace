"use strict";var util=require("util");var EventEmitter=require("events").EventEmitter;var Session_tcp=require("./session-tcp.js");var LF=String.fromCharCode(10);var CR=String.fromCharCode(13);var ACK=String.fromCharCode(6);var NAK=String.fromCharCode(21);var IDLE_TIME=300;function conv1(value){if(value==="ok"){return false}else if(value==="trb"){return true}return null}function str2pairs(str,deli1,deli2){if(!deli1){deli1=","}if(!deli2){deli2=":"}var pairs={};while(str.length>0){var i=str.indexOf(deli1);var field=str;if(i>0){field=str.substring(0,i);str=str.substring(i+1)}else{str=""}var name=field;var value=null;i=field.indexOf(deli2);if(i>0){name=field.substring(0,i);value=field.substring(i+1)}pairs[name]=value}return pairs}function Session_ipr_1(server,daemon,sessionId,socket){Session_tcp.call(this,server,daemon,sessionId,socket);this.package_buffer="";this.idle_timer_counter=0;this.send_cmd("GPS ACP",false)}util.inherits(Session_ipr_1,Session_tcp);Session_ipr_1.prototype.on_package=function(data){var cmds=data.split(",");var cmd=cmds.shift();var param=cmds;var ack=CR+LF+"UFOP RCV"+CR+LF;this.log(">==["+ack+"]");this.socket.write(ack);this.on_dev_cmd(cmd,param)};Session_ipr_1.prototype.on_dev_cmd_W=function(param){this.runtime.profile.tid=param[0];var cid=param[4].substring(4);var ER=cid.substring(2,3);var ercode=cid.substring(3,6);var GG=cid.substring(6,8);var CU="";var cucode=cid.substring(8,11);if(ER==="1"){ER="E"}else if(ER==="3"){ER="R"}if(ercode[0]==="4"){CU="U"}else{CU="C"}var event={};event.time=new Date;event.time_src="server";event.eid="";event.cid=this.acct()+" "+"18"+" "+ER+ercode+" "+GG+" "+CU+cucode;event.caller=this.sn();this.log("[cid event]",event.cid);this.emit("cid-event",this,event)};Session_ipr_1.prototype.on_dev_cmd_COWN=function(param){this.runtime.profile.tid=param[0];this.runtime.profile.model=param[1]};Session_ipr_1.prototype.on_dev_cmd_STP=function(param){this.runtime.profile.sn=param[0];var acct;try{acct=parseInt(param[8],16)}catch(err){}var new_acct=false;if(!this.acct()&&acct){new_acct=true}this.runtime.areas["01"].acct=acct;if(new_acct){this.emit("new-acct",this)}};Session_ipr_1.prototype.on_dev_cmd_MSG=function(param){var event={session:this,cid:"",caller:"",time:null};if(param.length<4){return}this.runtime.profile.sn=param[0];event.time=param[1];event.time_src="dev";var cid=param[3];var acct=parseInt(cid.substring(0,4),16);this.runtime.areas["01"].acct=acct;cid=cid.substring(4);var ER=cid.substring(2,3);var ercode=cid.substring(3,6);var GG=cid.substring(6,8);var CU=cid.substring(8,9);var cucode=cid.substring(9,12);event.eid="";event.cid=this.acct()+" "+"18"+" "+ER+ercode+" "+GG+" "+CU+cucode;event.caller=this.sn();this.log("[cid event]",event.cid);this.emit("cid-event",this,event)};Session_ipr_1.prototype.on_dev_cmd=function(cmd,param){this.idle_timer_counter=0;if(cmd==="MSG"){this.on_dev_cmd_MSG(param)}else if(cmd==="COWN"){this.on_dev_cmd_COWN(param)}else if(cmd==="W"){this.on_dev_cmd_W(param)}else if(cmd==="STP"){this.on_dev_cmd_STP(param)}else if(cmd==="STS"){}else if(cmd==="ACT"){}else if(cmd==="STN"){}else if(cmd==="V"){}else if(cmd==="W"){}};Session_ipr_1.prototype.do_process_buffer=function(){while(this.buffer.length>0){var ch=this.buffer[0];this.buffer=this.buffer.substring(1);if(ch==="$"){this.package_buffer=""}else if(ch==="#"){if(this.package_buffer.length>0){this.log("<==["+this.package_buffer+"]");this.on_package(this.package_buffer);this.package_buffer=""}}else{this.package_buffer=this.package_buffer+ch}}};Session_ipr_1.prototype.send_cmd=function(command,is_package){this.log(command);var cmd;if(is_package){cmd="$"+command+"#"}else{cmd=CR+LF+command+CR+LF}this.socket.write(cmd);this.log("==>["+cmd+"]")};module.exports=Session_ipr_1;