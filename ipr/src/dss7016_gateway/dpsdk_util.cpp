
#include "dpsdk_util.h"

/* 
   自动注册设备以注册ID(regDeviceCode)为唯一标识
   1.设备注册ID长度小于6,则前面补0
   2.设备注册ID长度不小于6,则取后6位
   注册ID配合LOCL-DH前缀构成LOCL-DHX-XX-XXX格式TID
   非自动注设备暂不支持
*/
const char *tid_pattern = "^[A-Za-z0-9]+$";

int DeviceChangeCallback_Func (IN int32_t nPDLLHandle,
                               IN uint32_t nChangeType,
                               IN const char *szDeviceId,
                               IN const char *szDepCode,
                               IN const char *szNewDepCode,
                               IN void *pUserParam) {
    List **list_device_change = (List **)pUserParam;
    
    Device_Event *event = (Device_Event *)calloc(1, sizeof(Device_Event));
    memcpy(event->device_id, szDeviceId, strlen(szDeviceId));

    switch (nChangeType) {
    case DPSDK_CORE_CHANGE_ADD_DEV:            // 增加设备
        event->event_type = DEVICE_CHANGE_ADD;
        list_push(*list_device_change, event);
        break;
    case DPSDK_CORE_CHANGE_MODIFY_DEV:         // 修改设备
        event->event_type = DEVICE_CHANGE_MODIFY;
        list_push(*list_device_change, event);
        break;
    case DPSDK_CORE_CHANGE_DEL_DEV:            // 删除设备
        event->event_type = DEVICE_CHANGE_DEL;
        list_push(*list_device_change, event);
        break;
    }
}

int DeviceStatusCallback_Func (IN int32_t nPDLLHandle,
                               IN const char *szDeviceId,
                               IN int32_t nStatus,
                               IN void *pUserParam) {

    List **list_device_status = (List **)pUserParam;

    Device_Event *event = (Device_Event *)calloc(1, sizeof(Device_Event));
    memcpy(event->device_id, szDeviceId, strlen(szDeviceId));
        
    switch (nStatus) {
    case DPSDK_CORE_STATUS_SERVER_ONLINE:    //设备上线
        event->event_type = DEVICE_STATUS_ONLINE;
        list_push(*list_device_status, event);
        break;
    case DPSDK_CORE_STATUS_SERVER_OFFLINE:   //设备下线
        event->event_type = DEVICE_STATUS_OFFLINE;
        list_push(*list_device_status, event);
        break;
    }
}

int DPSDKStatusCallback ( IN int32_t nPDLLHandle,
                          IN int32_t nStatus,
                          IN void *pUserParam ) {

    /*
      DPSDK_CORE_STATUS_SERVER_ONLINE     = 1,//服务上线通知
      DPSDK_CORE_STATUS_SERVER_OFFLINE    = 2,//服务下线通知
    */
    switch (nStatus) {
    case DPSDK_CORE_STATUS_SERVER_ONLINE:
        *(int *)pUserParam = 1;
        printf("DPSDK Status Server Online\n");
        break;
    case DPSDK_CORE_STATUS_SERVER_OFFLINE:
        *(int *)pUserParam = 0;
        printf("DPSDK Status Server Offline\n");
        break;
    }
}

DPSDK_Utils::DPSDK_Utils() {
    nDLLHandle         = -1;
    dss_server_status  = -1;
    list_device_change = NULL;
    list_device_status = NULL;
}

DPSDK_Utils::~DPSDK_Utils() {

}

void DPSDK_Utils::set_device_callback() {
    DPSDK_SetDPSDKDeviceChangeCallback(nDLLHandle, DeviceChangeCallback_Func, &list_device_change);
    DPSDK_SetDPSDKDeviceStatusCallback(nDLLHandle, DeviceStatusCallback_Func, &list_device_status);
    
}

int DPSDK_Utils::init_and_login() {
    list_device_change = list_create();
    list_device_status = list_create();
    
    if (list_device_change == NULL || list_device_status == NULL) {
        printf("list_device_change or list_device_status is NULL\n");
        return -1;
    }
    
	int nRet = DPSDK_Create(DPSDK_CORE_SDK_SERVER, nDLLHandle);
    if (nRet != DPSDK_RET_SUCCESS) {
        printf("DPSDK_Create Error! ErrCode: %d\n", nRet);
        return -2;
    }
    
    //注册平台状态(服务上线,服务下线)回调
    nRet = DPSDK_SetDPSDKStatusCallback(nDLLHandle, DPSDKStatusCallback, &dss_server_status);
    if (nRet != DPSDK_RET_SUCCESS) {
        printf("DPSDK_SetDPSDKStatusCallback Error! ErrCode: %d\n", nRet);
        return -3;
    }    

    Login_Info_t stuLoginInfo = {0};
    strcpy(stuLoginInfo.szIp,       DSS7016_LAN_IP);
    strcpy(stuLoginInfo.szUsername, DSS7016_SERVER_USERNAME);
    strcpy(stuLoginInfo.szPassword, DSS7016_SERVER_PASSWORD);

    stuLoginInfo.nPort     = DSS7016_SERVER_PORT;
    stuLoginInfo.nProtocol = DPSDK_PROTOCOL_VERSION_II;

    nRet = DPSDK_Login(nDLLHandle, &stuLoginInfo);
    if (DPSDK_RET_SUCCESS != nRet) {
        printf("DPSDK_Login Error! ErrCode: %d\n", nRet);
        return -4;
    }

    if (0 != regcomp(&tid_regex, tid_pattern, REG_EXTENDED)) {
        printf("TID Regex Compile Error!\n");
        return -5;
    }

    dss_server_status = 1;
    
    return 0;
}

int DPSDK_Utils::logout_and_destroy() {
    dss_server_status  = -1;

    list_clear_destroy(list_device_change);
    list_clear_destroy(list_device_status);

    regfree(&tid_regex);
    
    if (DPSDK_RET_SUCCESS != DPSDK_Logout(nDLLHandle)) {
        DPSDK_Destroy(nDLLHandle);
        return -1;
    }

    DPSDK_Destroy(nDLLHandle);
    
    return 0;
}

int DPSDK_Utils::get_channels_from_device_node(xmlNodePtr deviceNodePtr) {
    if (deviceNodePtr == NULL) {
        return 0;
    }
    int channels = 0;    
    xmlNodePtr nodePtr = deviceNodePtr->xmlChildrenNode;
    while (nodePtr) {
        if (nodePtr->type != XML_ELEMENT_NODE) {
            nodePtr = nodePtr->next;
            continue;
        }
        xmlChar *nodeName    = (xmlChar *)nodePtr->name;
        if (0 == xmlStrcmp(nodeName, (xmlChar *)"UnitNodes")) {
            xmlNodePtr channelNodePtr = nodePtr->xmlChildrenNode;
            while (channelNodePtr) {
                if (channelNodePtr->type != XML_ELEMENT_NODE) {
                    channelNodePtr = channelNodePtr->next;
                    continue;
                }
                channels ++;
                channelNodePtr = channelNodePtr->next;
            }
        }
        nodePtr = nodePtr->next;
    }
    return channels;
}

int DPSDK_Utils::is_lan_ip(char *ip) {
    if (ip == NULL || strlen(ip) == 0) {
        return -1;
    }
    char *field_1  = strsep(&ip, ".");
    char *field_2  = strsep(&ip, ".");

    int first  = atoi(field_1);
    int second = atoi(field_2);

    /*
      局域网IP地址段
      10.0.0.0-10.255.255.255
      172.16.0.0—172.31.255.255
      192.168.0.0-192.168.255.255
    */
    
    if ((first == 10) ||
        ((first == 172) && (second >= 16) && (second <= 31)) ||
        ((first == 192) && (second == 168))) {
        return 0;
    }
        
    return -1;
}

/*
<JRuntime> = {
    profile : <JProfile>,  // 基本信息
    devs    : <JDevs>,     // 设备信息
    areas   : <JAreas>,    // 报警分区信息
    zones   : <JZones>,    // 报警防区信息
    flags   : <JFlags>,    // 设备状态标记
    [parts] : <JParts>,    // 下级警云节点信息
}
*/
TObject *DPSDK_Utils::get_device_runtime_from_xml_node(xmlNodePtr nodePtr) {
    if (nodePtr == NULL) {
        return NULL;
    }

    /*
      DPSDK_Core_Define.h中dpsdk_dev_type_e结构体对应设备类型,视频设备的设备类型区间如下
      DEV_TYPE_ENC_BEGIN
      DEV_TYPE_ENC_END
     */
    char *attrType = (char *)xmlGetProp(nodePtr, (const xmlChar *)"type");
    int   type = atoi(attrType);
    if (type >= DEV_TYPE_ENC_END || type <= DEV_TYPE_ENC_BEGIN) {
        return NULL;
    }
    
    xmlChar *attrIP = xmlGetProp(nodePtr, (const xmlChar *)"ip");

    /* 
       自动注册设备的IP字段的值是DSS7016的IP,此时将自动注册ID(registDevicecode属生)转化为TID
    */
    char *orignCode = NULL;
    if (0 == xmlStrcmp(attrIP, (xmlChar *)DSS7016_LAN_IP)) {
        orignCode  = (char *)xmlGetProp(nodePtr, (xmlChar *)"registDeviceCode");
    } else {
        return NULL;
        //目前不支持非主动注册视频设备
        //orignCode  = (char *)xmlGetProp(nodePtr, (xmlChar *)"deviceSN");
    }

    /* 
       将视频设备的注册ID或序列号改为CONWIN的TID格式PRIV-DHX-XX-XXX 
       视频设备的注册ID如果小于6位则前面补0,如果大于6位则取后6位
    */
    int len = strlen(orignCode);
    char convertCode[10];
    if (len < 6) {
        memset(convertCode, '0', sizeof(convertCode));
        strncpy(convertCode + 6 - len, orignCode, len);
    } else {
        strncpy(convertCode, orignCode + len - 6, 6);
    }
    for (int i = 0; i < 6; i ++) {
        convertCode[i] = toupper(convertCode[i]); //字符小写转大写
    }
    convertCode[6] = 0;

    //判断注册ID是否符合规则
    if (0 != regexec(&tid_regex, convertCode, 0, NULL, 0)) {
        return NULL;
    }
    
    /* 将TID改为正确的格式 */
    char tid[30];
    sprintf(tid, "LOCL-DH%.1s-%.2s-%.3s", convertCode, convertCode + 1, convertCode + 3);

    char *attrId         = (char *)xmlGetProp(nodePtr, (xmlChar *)"id");
    char *attrStatus     = (char *)xmlGetProp(nodePtr, (xmlChar *)"status");
    char *attrName       = (char *)xmlGetProp(nodePtr, (xmlChar *)"name");
    char *attrPort       = (char *)xmlGetProp(nodePtr, (xmlChar *)"port");
    char *attrBrand      = (char *)xmlGetProp(nodePtr, (xmlChar *)"manufacturer");
    char *attrModel      = (char *)xmlGetProp(nodePtr, (xmlChar *)"model");
    char *attrUser       = (char *)xmlGetProp(nodePtr, (xmlChar *)"user");
    char *attrPass       = (char *)xmlGetProp(nodePtr, (xmlChar *)"password");
    char *attrDeviceIp   = (char *)xmlGetProp(nodePtr, (xmlChar *)"deviceIp");
    char *attrDevicePort = (char *)xmlGetProp(nodePtr, (xmlChar *)"devicePort");

    TObject *device_info = new TObject();
    device_info->new_member("id")->set(attrId);
    device_info->new_member("tid")->set((char *)tid);
    device_info->new_member("newtid")->set((char *)tid);
    device_info->new_member("addr")->set((char *)NULL);
    device_info->new_member("sid")->set((char *)NULL);
    device_info->new_member("agentPort")->set(atoi(attrPort));
    device_info->new_member("deviceIP")->set(attrDeviceIp);
    device_info->new_member("devicePort")->set(atoi(attrDevicePort));
    device_info->new_member("regCode")->set(orignCode);
    device_info->new_member("type")->set(attrType);
    device_info->new_member("name")->set(attrName);
    device_info->new_member("logined")->set(T_FALSE);
    /* 在线状态(1在线,2离线) */
    device_info->new_member("status")->set(atoi(attrStatus));
    /* 旧登录IP,内网设备为设备真实IP,外网设备为DSS7016外网IP,此种只适用于手机APP和7016在一个网 */
    /* 新登录IP,手机APP和7016可能不再一个网,登录IP固定为7016外网IP */
    char ip[50] = {0};
    strcpy(ip, attrDeviceIp);
    // if (0 == is_lan_ip((char *)ip)) {
    //     device_info->new_member("loginIP")->set(attrDeviceIp);
    // } else {
        device_info->new_member("loginIP")->set((char *)DSS7016_WAN_IP);
    // }
    
    /* runtime需要上报到警云 */
    TObject *runtime = device_info->new_member("runtime");
    TObject *profile = runtime->new_member("profile");
    profile->new_member("tid")->set((char *)tid);
    profile->new_member("pid")->set(2000);
    profile->new_member("brand")->set(attrBrand);
    profile->new_member("model")->set(attrModel);
    profile->new_member("name")->set(attrName);
    profile->new_member("sdk")->set((char *)NULL);

    TObject *devs = runtime->new_member("devs");
    devs->new_member("self")->set((char *)NULL);

    TObject *videos = devs->new_member("videos");
    int channels = get_channels_from_device_node(nodePtr);
    for (int i  = 0; i < channels; i ++) {
        char buf[100];
        sprintf(buf, "ch%d", i + 1);
        TObject *ch = videos->new_member(buf);
        ch->new_member("source");
    }

    TObject *video_dev_default = videos->new_member("default");
    video_dev_default->new_member("capture")->set(T_FALSE);
    TObject *access = video_dev_default->new_member("access");
    access->new_member("protocol")->set("dahua");
    access->new_member("ip")->set((char *)DSS7016_LAN_IP);
    access->new_member("port")->set(atoi(attrPort));
    access->new_member("upnp-port")->set(0);
    access->new_member("user")->set(attrUser);
    access->new_member("pass")->set(attrPass);
    TObject *p2p = access->new_member("p2p");
    p2p->new_member("enabled")->set(T_FALSE);
    p2p->new_member("server")->set("p2p.conwin.cc");

    
    runtime->new_member("areas")->set((char *)NULL);
    runtime->new_member("parts")->set((char *)NULL);
    TObject *flags = runtime->new_member("flags");
    flags->new_member("p2p-connected")->set(T_FALSE);
    flags->new_member("storage-fail")->set(T_FALSE);
    flags->new_member("storage-full")->set(T_FALSE);

    return device_info;
}


TObject *DPSDK_Utils::parse_devices_node(xmlNodePtr devicesNodePtr) {
    if (devicesNodePtr == NULL) {
        return NULL;
    }

    TObject   *devices_runtime = new TObject();
    xmlNodePtr nodePtr = devicesNodePtr->xmlChildrenNode;
    while (nodePtr) {
        if (nodePtr->type != XML_ELEMENT_NODE) {
            nodePtr = nodePtr->next;
            continue;
        }
        
        TObject *device_info = get_device_runtime_from_xml_node(nodePtr);
        if (device_info != NULL) {
            char *attrId         = (char *)xmlGetProp(nodePtr, (xmlChar *)"id");
            devices_runtime->new_member(attrId)->assign(device_info);
        }
        
        nodePtr = nodePtr->next;
    }
    return devices_runtime;
}


xmlNodePtr DPSDK_Utils::get_xml_devices_node_ptr(xmlDocPtr docPtr) {
    if (docPtr == NULL) {
        printf("docPtr is NULL!\n");
        return NULL;
    }

    xmlNodePtr rootPtr = xmlDocGetRootElement(docPtr);
    if (rootPtr == NULL) {
        printf("xml get root element error!\n");
        xmlFreeDoc(docPtr);
        return NULL;
    }

    xmlNodePtr nodePtr = rootPtr->xmlChildrenNode;
    while (nodePtr) {
        if (nodePtr->type == XML_ELEMENT_NODE) {
            xmlChar *nodeName    = (xmlChar *)nodePtr->name;
            if (0 == xmlStrcmp(nodeName, (xmlChar *)"Devices")) {
                return nodePtr;
            }
        }
        nodePtr = nodePtr->next;
    }

    return NULL;
}

char *DPSDK_Utils::load_group_info() {
    int nGroupLen;
    int nRet = DPSDK_LoadDGroupInfo(nDLLHandle, nGroupLen);
    if (DPSDK_RET_SUCCESS != nRet ) {
        printf("Load Group Info Error! ErrCode: %d\n", nRet);
        return NULL;
    }
    
    //printf("Load Group Info Success! nGroupLen: %d\n", nGroupLen);
    char *buf = (char *)malloc(nGroupLen + 1);
    nRet = DPSDK_GetDGroupStr(nDLLHandle, buf, nGroupLen);
    if (nRet != DPSDK_RET_SUCCESS) {
        printf("Get Group Str Error! ErrCode: %d\n", nRet);
        return NULL;
    }

    buf[nGroupLen] = 0;
    return buf;
}

TObject *DPSDK_Utils::get_all_devices_runtime() {
    char *buf = load_group_info();
    if (buf == NULL) {
        return NULL;
    }
    xmlDocPtr docPtr = xmlParseMemory(buf, strlen(buf));
    xmlNodePtr devicesNodePtr = get_xml_devices_node_ptr(docPtr);
    TObject *all_devices_runtime = parse_devices_node(devicesNodePtr);
    
    xmlFreeDoc(docPtr);
    free(buf);
    
    return all_devices_runtime;
}


xmlNodePtr DPSDK_Utils::get_xml_node_ptr_by_id(xmlNodePtr devicesNodePtr, const char *device_id) {
    if (devicesNodePtr == NULL || device_id == NULL) {
        return NULL;
    }
    
    xmlNodePtr nodePtr = devicesNodePtr->xmlChildrenNode;
    while (nodePtr) {
        if (nodePtr->type != XML_ELEMENT_NODE) {
            nodePtr = nodePtr->next;
            continue;
        }
        xmlChar *attrId    = xmlGetProp(nodePtr, (xmlChar *)"id");
        if (0 == xmlStrcmp(attrId, (xmlChar *)device_id)) {
            return nodePtr;
        }

        nodePtr = nodePtr->next;
    }

    return NULL;
}


TObject *DPSDK_Utils::get_device_runtime_by_id(char *device_id) {
    if (device_id == NULL) {
        return NULL;
    }

    char *buf = load_group_info();
    xmlDocPtr  docPtr = xmlParseMemory(buf, strlen(buf));
    xmlNodePtr devicesNodePtr = get_xml_devices_node_ptr(docPtr);
    xmlNodePtr nodePtr = get_xml_node_ptr_by_id(devicesNodePtr, (const char *)device_id);
    if (nodePtr == NULL) {
        printf("not found nodePtr by id\n");
        return NULL;
    }

    TObject *runtime = get_device_runtime_from_xml_node(nodePtr);

    xmlFreeDoc(docPtr);
    free(buf);

    return runtime;
}


// void test_device() {
    
    // char *buf = get_xml_buffer_from_server();
    // xmlDocPtr docPtr = xmlParseMemory(buf, strlen(buf));
    // xmlNodePtr devicesNodePtr = get_xml_devices_node_ptr(docPtr);

    // xmlNodePtr nodePtr = devicesNodePtr->xmlChildrenNode;
    // while (nodePtr) {
    //     if (nodePtr->type != XML_ELEMENT_NODE) {
    //         nodePtr = nodePtr->next;
    //         continue;
    //     }
    //     xmlChar *attrIP   = xmlGetProp(nodePtr, (const xmlChar *)"ip");
    //     xmlChar *attrName = xmlGetProp(nodePtr, (const xmlChar *)"name");
    //     if (0 == xmlStrcmp(attrIP, (xmlChar *)DSS7016_SERVER_IP)) {
    //         printf("%s, %s\n", attrIP, attrName);
    //         cnt ++;
    //     }
    //     nodePtr = nodePtr->next;
    // }

    //TObject *all_device_runtime = parse_devices_node(devicesNodePtr);
    // for (int i = 0; i < all_device_runtime->member_cnt(); i ++) {
    //     printf("%s\n", all_device_runtime->member(i)->to_string());
    // }

    // TObject *device = all_device_runtime->member("1000107");
    // if (device) {
    //     device->new_member("addr")->set("xx.xxxxxx");
    //     printf("addr: %s\n", device->to_string("addr"));
    // }
    
    // xmlFreeDoc(docPtr);
    // free(buf);

    //printf("cnt: %d\n", cnt);

    //TObject *all_devices_runtime = get_all_devices_runtime();
    //printf("%s\n", all_devices_runtime->to_string());
    //delete all_devices_runtime;

    // TObject *runtime = get_device_runtime_by_id("1000102");
    // printf("%s\n", runtime->to_string());
    // delete runtime;
    // return;
    
    // while (true) {

    //     if (list_device_change->count > 0) {

    //         DPSDK_Callback_Event *event = (DPSDK_Callback_Event *)list_shift(list_device_change);
    //         if (event) {
    //             printf("event->id: %s\n",   event->device_id);
    //             printf("event->type: %d\n", event->event_type);
    //             free(event);
    //         }

    //     }

    //     if (list_device_status->count > 0) {

    //         DPSDK_Callback_Event *event = (DPSDK_Callback_Event *)list_shift(list_device_status);
    //         if (event) {
    //             printf("event->id: %s\n",   event->device_id);
    //             printf("event->type: %d\n", event->event_type);
    //             free(event);
    //         }

    //     }

    // }

    // TObject *all = get_all_devices_runtime();
    // for (int i = 0; i < all->member_cnt(); i ++) {
    //     TObject *runtime = all->member(i);
    //     printf("%s, %s\n", runtime->member("id")->to_string(), runtime->member("tid")->to_string());
    // }

    // printf("cnt: %d\n", all->member_cnt());

    // delete all;
// }

