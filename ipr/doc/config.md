
参数配置文件定义
================

```
    config.json = {
        reciver-number       : <number>  // 1-8          default: 1      接收机号
        serial-format        : <string>  // 685 | conwin default: conwin 串口输出格式
        serial-baud          : <number>  // 9600 - 115200 default: 38400 串口输出速度
        panel-event-first    : <boolean> // true | false default: true   优先上传主机事件
        report-online-offline: <boolean> // true | false default: true   模块上线/掉线时报告事件
        log-online-offline   : <boolean> // true | false default: true   日志文件包含模块上线/掉线事件
        confirm-offline-time : <number>  // 0 - 600      default: 60     断线认定时间(秒)
        allow-control-device : <boolean> // true | false default: true   允许向设备发送控制命令
        api-key              : <string>  // max len = 30 default: "1234" API访问密码
        api-allow-set-time   : <boolean> // true | false default: true   允许通过API设置时间
        api-allow-rebot      : <boolean> // true | false default: true   允许通过API重启
        web-admin-port       : <number>  // 2000 - 10000 default: 8000   管理界面端口
        lines : {
            <line number> : {
                enabled  : <boolean> // true | false     default : true
                name     : <string>  // length < 50      default : ""
                prefix   : <string>  // length < 4       default : "" // 用户号前缀
                port     : <number>  // 2000 - 10000     default : protocol depend
                protocol : <string> // "ipr-1", "ipr-2"  default : ipr-2
            }
        }
}
```
