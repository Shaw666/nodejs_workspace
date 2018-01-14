#ifndef _DEVICE_H
#define _DEVICE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "things/things.h"
#include "things/tobject.h"
#include "things/tparser.h"

#include "jnode_env.h"
#include "jnode_util.h"
#include "http_util.h"
#include "dpsdk_util.h"

#include "config.h"
#include "log.h"

class Device {

 private:

    Things *t;
    unsigned int loop_cnt;
    DPSDK_Utils *dpsdk_utils;
    void update_device_info_by_id(char *id, TObject *new_tobject);
    
 public:

    Device(Things *things);
    ~Device();

    bool dpsdk_init_flag;               
    bool things_server_connected;      //Things服务器状态
    bool dss_server_connected;         //DSS7016服务器状态
    bool devices_online_info_changed;  //在线设备信息是否改变
    TObject *all_devices_runtime;
    
    int  init();
    void login(char *id, char *tid, char *addr, char *raddr, int rport);
    void logout(char *id, char *tid, char *sid, char *addr);

    void push_full_runtime(char *id);
    void change_loop();
    void status_loop();
    void loop();
    void tick_1s();
    void all_devices_logout();
    void update_online_offline_event();
    void update_online_devices_info(unsigned int tim);
    void set_online_device_num(int num);
};

extern Device  *d;

#endif
