#ifndef _CONFIG_H
#define _CONFIG_H

/* DSS7016服务器IP,PORT,Username,Password */
extern char DSS7016_LAN_IP[128];
extern char DSS7016_WAN_IP[128];
extern char DSS7016_SERVER_USERNAME[128];
extern char DSS7016_SERVER_PASSWORD[128];
extern int  DSS7016_SERVER_PORT;

/* 虚拟设备自身TID, 警云服务器连接URL */
extern char SELF_TID[128];
extern char THINGS_SERVER[128];

int config_init();

#endif
