#include "device.h"

Device  *d = NULL;

Device::Device(Things *things) {
    t                           = things;
    loop_cnt                    = 0;
    dpsdk_init_flag             = false;
    devices_online_info_changed = false;
    things_server_connected     = false;
    dss_server_connected        = false;
    all_devices_runtime         = NULL;
    dpsdk_utils                 = new DPSDK_Utils();
}

Device::~Device() {
    if (all_devices_runtime) {
        delete all_devices_runtime;
    }
    if (dpsdk_utils) {
        if (dpsdk_init_flag) {
            dpsdk_utils->logout_and_destroy();
        }
        delete dpsdk_utils;
        printf("DPSDK Logout and Destroy OK!\n");
        LOG_INFO("DPSDK Logout and Destroy OK!");
    }
}

int Device::init() {

    /* SDK Initialize and Login DSS7016 */
    int res = dpsdk_utils->init_and_login();
    if (res != 0) {
        LOG_ERROR("DPSDK Init and Login Error, Error Code: %d", res);
        return -1;
    }

    /* Set Device and Status Change Callback */
    dpsdk_utils->set_device_callback();

    /* get all devices runtime  */
    all_devices_runtime = dpsdk_utils->get_all_devices_runtime();
    if (all_devices_runtime == NULL) {
        printf("Get All Devices Runtime Error!\n");
        LOG_TRACE("Get All Devices Runtime Error!");
        return -2;
    }

    dpsdk_init_flag = true;
    devices_online_info_changed = true;

    LOG_INFO("DPSDK Init and Login OK");

    return 0;
}

/*
  设备登录请求响应回调
*/
void on_device_login_response(Things *this_t, void *data, void *context){
    T_RESPONSE *res = (T_RESPONSE*)data;
    char  buf[1024];
    char *pbuf = (char *)buf;
    strncpy(pbuf, res->body, sizeof(buf) - 1);

    char *sid = strsep(&pbuf, ","); 
    char *id  = (char *)context;  

    printf("response: [%d], SID: %s, ID: %s\n", res->status_code, sid, id);
        
    /* res->status_code: 200 success 登录成功,获取session id */
    if ((200 == res->status_code) && (sid != NULL) && (0 != strcmp(sid, "undefined"))) { 
        LOG_INFO("Device Login Response Success, id: %s, sid: %s, res->body: %s", id, sid, res->body);
        if (d) {
            d->all_devices_runtime->member(id)->member("sid")->set(sid);
            d->all_devices_runtime->member(id)->member("logined")->set(T_TRUE);
            d->push_full_runtime(id);
            d->devices_online_info_changed = true;
        }
    } else {
        if (d) {
            d->all_devices_runtime->member(id)->member("sid")->set((char *)NULL);
            d->all_devices_runtime->member(id)->member("addr")->set((char *)NULL);
            d->all_devices_runtime->member(id)->member("logined")->set(T_FALSE);
        }
        //登录失败,记录日志
        LOG_ERROR("Device Login Response Fail, id: %s, res->code: %d, res->body: %s", id, res->status_code, res->body);
        
    }
}


/*
  设备登录
  调用方法：t->request('.', url, response_callback, context);
      url格式：/login?data=params   注意！其中的params参数面要url编码
          params格式：
              { 
                type: "tid", 
                tid:  "tid值", 
                addr: "设备地址", 
                userdata: {pid: 2000}
                raddr:
                rport:
              }
      response_callback: 登录请求响应回调
      context: 上下文参数
*/
void Device::login(char *id, char *tid, char *addr, char *raddr, int rport) {
    if (!things_server_connected) {
        return;
    }

    printf("login: %s, %s, %s, %s, %d\n", id, tid, addr, raddr, rport);
    
    TObject *params  = new TObject();
    params->new_member("type")->set("tid");
    params->new_member("tid")->set(tid);
    params->new_member("addr")->set(addr);
    params->new_member("remoteAddress")->set(raddr);
    params->new_member("remotePort")->set(rport);

    TObject *userdata = params->new_member("userdata");
    userdata->new_member("pid")->set(2000);

    char buf[512];
    int len = encodeURIComponent(params->to_string(), buf, sizeof(buf) - 1);
    buf[len] = 0;
            
    char url[1024];
    sprintf(url, "/login?data=%s", buf);
    delete params;

    LOG_INFO("device login info: %s, %s, %s, %s, %d", id, tid, addr, raddr, rport);
    t->request(".", url, on_device_login_response, (void *)id);
}


/*
  设备数据上传
  调用方法：t->pushEx(addr, ".", "v", msg)
    addr格式: "%s.6666%04d"   前面的%s是t->addr, 后面8位由自已生成
    "v"表示是var event
    msg格式："f, 0, %s"  f表示full(push整个JRuntime结构), 0是计数器, %s是整个JRuntime结构
*/
void Device::push_full_runtime(char *id) {
    if (id == NULL) {
        return ;
    }
    char    *addr    = all_devices_runtime->member(id)->member("addr")->to_string();
    TObject *runtime = all_devices_runtime->member(id)->member("runtime");
    if (runtime && addr) {
        char msg[1000];
        sprintf(msg, "f,0,%s", runtime->to_string());
        t->pushEx(addr, ".", "v", msg);
    }
}

/*
  设备下线(上线需要重新登录并push full runtime, 下线需要调用本方法)
  数据发送方法：t->pushEx(t->addr, ".", "e", msg);
  其中msg格式为:  "3/4, {tid: ; sid: ; addr: }"
*/
void Device::logout(char *id, char *tid, char *sid, char *addr) {

    TObject *obj = new TObject();
    obj->new_member("tid")->set(tid);
    obj->new_member("sid")->set(sid);
    obj->new_member("addr")->set(addr);

    /*
      online/offline event
      session connected    —— 3
      session disconnected —— 4
    */
    char msg[512];
    sprintf(msg, "4,%s", obj->to_string());
    delete obj;

    printf("Device Logout, id: %s\n", id);
    LOG_INFO("Device Logout, id: %s", id);

    t->pushEx(t->addr, ".", "e", msg);
}

void Device::update_device_info_by_id(char *id, TObject *new_tobject) {
    if (id == NULL) {
        return;
    }
    /*
      设备修改,旧的设备信息应先从警云logout,然后删除信息
    */
    TObject *old_device_info = all_devices_runtime->member(id);
    if (old_device_info) {
        bool logined = old_device_info->member("logined")->to_bool();
        if (logined) {
            char *tid  = old_device_info->member("tid")->to_string();
            char *sid  = old_device_info->member("sid")->to_string();
            char *addr = old_device_info->member("addr")->to_string();
            logout(id, tid, sid, addr);            
        }
        all_devices_runtime->del(id);
    }
    /*
      修改后的信息为合法信息,则重新添加到all_devices_runtime
    */
    if (new_tobject) {
        all_devices_runtime->new_member(id)->assign(new_tobject);
    }
}

/* 
   设备增,删,改 
   增：添加设备信息,发送上线信息
   删：删除设备信息,发送下线信息
   改: 修改设备信息,重新push full runtime
*/
void Device::change_loop() {
    if (dpsdk_utils->list_device_change->count == 0) {
        return;
    }

    if (!things_server_connected) {
        return;
    }

    Device_Event *event = (Device_Event *)list_shift(dpsdk_utils->list_device_change);
    if (event == NULL) {
        return;
    }

    char *device_id = event->device_id;
    int   event_type = event->event_type;
    printf("Device Change Event, id: %s, type: %d\n", device_id, event_type);
    LOG_INFO("Device Change Event, id: %s, type: %d\n", device_id, event_type);
    TObject *device_info = NULL;
    switch (event->event_type) {
    case DEVICE_CHANGE_ADD:        /* 设备添加,添加runtime */
        device_info = dpsdk_utils->get_device_runtime_by_id(device_id);
        if (device_info) {
            all_devices_runtime->new_member(device_id)->assign(device_info);
        }
        break;
    case DEVICE_CHANGE_MODIFY:    /* 设备修改,更新runtime */
        device_info = dpsdk_utils->get_device_runtime_by_id(device_id);
        if (device_info == NULL) {
            printf("Device Change Modify, Device new info is NULL\n");
            LOG_ERROR("Device Change Modify, New Info is NULL, id: %s\n", device_id);
        }
        update_device_info_by_id(device_id, device_info);
        break;
    case DEVICE_CHANGE_DEL:       /* 设备删除,删除runtime */
        device_info = all_devices_runtime->member(device_id);
        if (device_info) {
            char *tid    = device_info->member("tid")->to_string();
            char *sid    = device_info->member("sid")->to_string();
            char *addr   = device_info->member("addr")->to_string();
            bool logined = device_info->member("logined")->to_bool();
            if (logined) {
                logout(device_id, tid, sid,  addr);
            }
            all_devices_runtime->del(device_id); /* 删除设备信息 */
        }
        break;
    }
    loop_cnt = 0;
    devices_online_info_changed = true;

    free(event);
}

/* 
   设备状态改变
   设备上线/下线推送dev event 
*/
void Device::status_loop() {
    if (dpsdk_utils->list_device_status->count == 0) {
        return;
    }

    if (!things_server_connected) {
        return;
    }

    Device_Event *event = (Device_Event *)list_shift(dpsdk_utils->list_device_status);
    if (event == NULL) {
        return;
    }

    char *device_id   = event->device_id;
    int   event_type  = event->event_type;
    printf("Device Status Event, id: %s, type: %d\n", device_id, event_type);
    LOG_INFO("Device Status Event, id: %s, type: %d", device_id, event_type);

    TObject *device_info = all_devices_runtime->member(device_id);
    if (device_info) {
        char *id         = device_info->member("id")->to_string();
        char *tid        = device_info->member("tid")->to_string();
        char *newtid     = device_info->member("newtid")->to_string();
        char *addr       = device_info->member("addr")->to_string();
        char *sid        = device_info->member("sid")->to_string();
        char *loginIP    = device_info->member("loginIP")->to_string();
        int   devicePort = device_info->member("devicePort")->to_int();
        bool  logined    = device_info->member("logined")->to_bool();
        
        /* 更新设备在线状态 */
        device_info->member("status")->set(event_type);

        switch (event_type) {
        case DEVICE_STATUS_OFFLINE: /* 如果设备已经连接,则需要logou t*/
            if (logined) {
                logout(device_id, tid, sid,  addr);
                device_info->member("sid")->set((char *)NULL);
                device_info->member("addr")->set((char *)NULL);
                device_info->member("logined")->set(T_FALSE);
            }
            break;
        case DEVICE_STATUS_ONLINE:
            device_info->member("tid")->set(newtid);
            TObject *taddr = device_info->member("addr");
            if (taddr->type() == T_NULL_VALUE) {
                char addr[50];
                int agentPort = device_info->member("agentPort")->to_int();
                sprintf(addr, "%s.000%05d", t->addr, agentPort);
                taddr->set((char *)addr);
            }
            login(id, tid, taddr->to_string(), loginIP, devicePort);
            break;
        }
    }

    loop_cnt = 0;
    devices_online_info_changed = true;

    free(event);
}

void Device::loop() {
    if (dss_server_connected) {
        change_loop();
        status_loop();
    }
}

void Device::update_online_devices_info(unsigned int tim) {
    if (all_devices_runtime == NULL) {
        return;
    }

    loop_cnt ++;
    
    static unsigned int last_number = 0;
    unsigned int online_number      = 0;
 
    TObject *obj = new TObject();
    TObject *online_devices = obj->new_member("online-devices");

    unsigned int member_cnt = all_devices_runtime->member_cnt();
    for (int i = 0; i < member_cnt; i ++) {
        TObject *device_info = all_devices_runtime->member(i);
        if (device_info == NULL) {
            continue;
        }
        /* 设备在线信息,1在线,2离线 */
        int status = device_info->member("status")->to_int();
        if (status == DEVICE_STATUS_ONLINE) {
            char *id        = device_info->member("id")->to_string();
            char *sid       = device_info->member("sid")->to_string();
            char *tid       = device_info->member("tid")->to_string();
            char *deviceIP  = device_info->member("deviceIP")->to_string();
            char *name      = device_info->member("name")->to_string();
            char *type      = device_info->member("type")->to_string();
            char *regCode   = device_info->member("regCode")->to_string();
            char *loginIP   = device_info->member("loginIP")->to_string();
            int  devicePort = device_info->member("devicePort")->to_int();
            int  agentPort  = device_info->member("agentPort")->to_int();
            
            /* 如果还未登录或登录失败,则10重新登录 */
            bool logined  = device_info->member("logined")->to_bool();
            if (!logined && loop_cnt > 60) {
                TObject *taddr = device_info->member("addr");
                if (taddr->type() == T_NULL_VALUE) {
                    char addr[50];
                    sprintf(addr, "%s.000%05d", t->addr, agentPort);
                    taddr->set((char *)addr);
                }

                login(id, tid, taddr->to_string(), loginIP, devicePort);

                devices_online_info_changed = true;
            }

            /* 更新在线设备信息 */
            TObject *d = new TObject();
            d->new_member("id")->set(id);
            d->new_member("tid")->set(tid);
            d->new_member("deviceIP")->set(deviceIP);
            d->new_member("agentPort")->set(agentPort);
            d->new_member("name")->set(name);
            d->new_member("type")->set(type);
            d->new_member("regCode")->set(regCode);
            d->new_member("logined")->set(logined);

            online_devices->push(d);
            online_number ++;
        }
    }

    if ((last_number != online_number) || devices_online_info_changed) {
        last_number = online_number;
        
        obj->new_member("online-number")->set((int)online_number);
        char path[128];
        sprintf(path, "%s/gateway-runtime.stat", ENV_JRAM);
        write_json_file(path, obj);
    }

    delete obj;
}

void Device::all_devices_logout() {
    if (all_devices_runtime == NULL) {
        return;
    }
    unsigned int member_cnt = all_devices_runtime->member_cnt();
    for (int i = 0; i < member_cnt; i ++) {
        TObject *device_info = all_devices_runtime->member(i);
        if (device_info == NULL) {
            continue;
        }
        bool status  = device_info->member("status")->to_int();
        bool logined = device_info->member("logined")->to_bool();
        if (logined && (status == DEVICE_STATUS_ONLINE)) {
            char *id   = device_info->member("id")->to_string();
            char *tid  = device_info->member("tid")->to_string();
            char *sid  = device_info->member("sid")->to_string();
            char *addr = device_info->member("addr")->to_string();
            logout(id, tid, sid, addr);
            device_info->member("logined")->set(T_FALSE);
        }
    }
}

void Device::update_online_offline_event() {
    //DSS7016上线
    if ((dss_server_connected == false) &&
        (dpsdk_utils->dss_server_status == 1)) {
        dss_server_connected = true;
        if (!dpsdk_init_flag) {
            dpsdk_utils->logout_and_destroy();
            if (0 == init()) {
                jnode_set_flag("dss7016-server-connected", true);
            }
        }
        LOG_INFO("DSS7016 Server Online");
    }

    //DSS7016下线
    if ((dss_server_connected == true) &&
        (dpsdk_utils->dss_server_status == 0)) {
        dss_server_connected = false;
        if (all_devices_runtime) {
            all_devices_logout();
            delete all_devices_runtime;
            all_devices_runtime = NULL;
            set_online_device_num(0);
            jnode_set_flag("dss7016-server-connected", false);
        }
        dpsdk_init_flag = false;
        LOG_INFO("DSS7016 Server Offline");
    }
}

void Device::tick_1s() {
    static unsigned int tim = 0;

    update_online_offline_event();

    loop_cnt ++;
    
    //5S更新一次在线设备数量信息
    if (++tim % 5 == 0) {
        update_online_devices_info(tim);
    }
}

void Device::set_online_device_num(int num) {
    // if (num == 0) {
    //     all_devices_logout();
    // }
    
    TObject *obj = new TObject();
    obj->new_member("online-number")->set((int)num);
    obj->new_member("online-devices")->set((char *)NULL);
    
    char path[128];
    sprintf(path, "%s/gateway-runtime.stat", ENV_JRAM);
    write_json_file(path, obj);
    delete obj;
}
