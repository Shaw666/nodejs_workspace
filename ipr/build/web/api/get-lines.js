"use strict";module.exports=function(req,res){var result=req.app.server.config.ipr["tcp-lines"];res.end(JSON.stringify(result))};