﻿版本更新

##### * build-version="0.4.11.0"： 添加视频网关模块CN0802
>* 网关模块作为CN8010接警机的一个子模块,型号CN0802
>* 网关模块连接DSS7016服务器,获取在线的主动注册的视频设备信息
>* 网关模块作为警云子节点连接警云服务器,将获取的视频设备信息以2000设备格式上报到警云

>* Web页面添加配置DSS7016服务器的LAN IP，端口，登录名，密码，WAN IP；
>* Web页面添加配置警云服务器的IP，端口的配置以及是否启用视频网关功能
>* Web页面添加视频网关模块的TID，DSS7016服务器和警云服务器连接状态，在线视频设备数以及在线视频设备列表

##### * alpha-version="0.4.10.0"：优化了300T模块上传报警事件的编号“A”转换为“0”
##### * alpha-version="0.4.9.0"： 增加了685通讯协议,是否上传OBO和0B2的状态信息的开关
##### * alpha-version="0.4.7.0"： 修复附加模块CN0801/0802不能正确添加的bug
##### * alpha-version="0.4.6.0"： 修复用户编号包含A-F字母时中心软件不能反控的bug

##### * alpha-version="0.4.4.0"： 添加CN4008主机可编程输出控制功能
##### * alpha-version="0.4.3.0"： 稳定版本
