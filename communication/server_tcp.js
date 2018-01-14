var net = require('net');
var clients = 0;

var server = net.createServer(function(connection) {
    console.log('有TCP客户端连接进入');
    clients++;
    var clientId = clients;
    console.log('Client connected:',clientId);
    client.on('end', function(){
	console.log('Client disconnected:', clientId);
    });

    client.write('Welcome client: '+clientId+'rn');
    client.pipe(client);
});

//TCP服务器开始端口监听
server.listen(8000, function() {
    console.log('TCP服务启动');
});
