#ifndef _DPSDK_UTILS_H
#define _DPSDK_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

#include "DPSDK/DPSDK_Core.h"
#include "DPSDK/DPSDK_Core_Define.h"
#include "DPSDK/DPSDK_Core_Error.h"

#include "libxml/parser.h"
#include "libxml/tree.h"
#include "libxml/xpath.h"

#include "things/tobject.h"

#include "list.h"

#include "config.h"

enum DEVICE_CHANGE_EVENT {
    DEVICE_CHANGE_UNDEF = 0,
    DEVICE_CHANGE_ADD,
    DEVICE_CHANGE_MODIFY,
    DEVICE_CHANGE_DEL,
};

enum DEVICE_STATUS_EVENT {
    DEVICE_STATUS_ONLINE = 1,
    DEVICE_STATUS_OFFLINE,
};

typedef struct _Device_Event {
    char device_id[32];
    int  event_type;
} Device_Event;


class DPSDK_Utils {
 private:

    int nDLLHandle;                /* DPSDK句柄 */
    regex_t tid_regex;             /* 正则表达式对象 */

    xmlNodePtr get_xml_devices_node_ptr(xmlDocPtr docPtr);
    xmlNodePtr get_xml_node_ptr_by_id(xmlNodePtr devicesNodePtr, const char *device_id);
    char *load_group_info();
    int get_channels_from_device_node(xmlNodePtr deviceNodePtr);

    TObject *parse_devices_node(xmlNodePtr devicesNodePtr);
    TObject *get_device_runtime_from_xml_node(xmlNodePtr nodePtr);

    int is_lan_ip(char *ip);       //判断是否为局域网IP
    
 public:

    DPSDK_Utils();
    ~DPSDK_Utils();
    
    int  dss_server_status;        //DSS7016服务器状态(1在线,0离线,-1未知)
    
    List *list_device_change;      //设备增,删,改
    List *list_device_status;      //设备状态改变(上线,下线)

    int  init_and_login();         
    int  logout_and_destroy();

    void set_service_callback();   //平台状态(服务上线,服务下线)回调
    void set_device_callback();    //设备change和status回调
    
    TObject *get_all_devices_runtime();
    TObject *get_device_runtime_by_id(char *device_id);
    
};

#endif
