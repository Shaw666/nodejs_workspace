#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "jnode_env.h"
#include "jnode_util.h"
#include "things/tobject.h"
#include "things/cwlib.h"
#include "things/tparser.h"
#include "log.h"

char DSS7016_LAN_IP[128]          = {0};
char DSS7016_WAN_IP[128]          = {0};
char DSS7016_SERVER_USERNAME[128] = {0};
char DSS7016_SERVER_PASSWORD[128] = {0};
int  DSS7016_SERVER_PORT          = 0;

char SELF_TID[128]                = {0};
char THINGS_SERVER[128]           = {0};

int config_init()
{
    if (jnode_env_init() < 0) {
        return -1;
    }

    // printf("ENV_JROOT: %s\n", ENV_JROOT);
    // printf("ENV_JSYS: %s\n",  ENV_JSYS);
    // printf("ENV_JTMP: %s\n",  ENV_JTMP);
    // printf("ENV_JRAM: %s\n",  ENV_JRAM);
    // printf("ENV_JFLAGS: %s\n", ENV_JFLAGS);
    // printf("ENV_JVAR: %s\n", ENV_JFLAGS);
    // printf("ENV_APP_HOME: %s\n", ENV_APP_HOME);
    // printf("ENV_APP_NAME: %s\n", ENV_APP_NAME);

    char config_file_path[128];
    sprintf(config_file_path, "%s/etc/gateway.json", ENV_JSYS);
    TObject *config = load_json_file((const char *)config_file_path);
    if (!config) {
        return -2;
    }

    bool  gateway_enabled     = config->member("gateway-enabled")->to_bool();

    char *dss7016_lan_ip      = config->member("dss7016-lan-ip")->to_string();
    char *dss7016_wan_ip      = config->member("dss7016-wan-ip")->to_string();
    int   dss7016_server_port = config->member("dss7016-server-port")->to_int();
    char *dss7016_username    = config->member("dss7016-username")->to_string();
    char *dss7016_password    = config->member("dss7016-password")->to_string();    

    char *things_server_ip    = config->member("things-server-ip")->to_string();
    int   things_server_port  = config->member("things-server-port")->to_int();
    char *gateway_device_tid  = config->member("tid")->to_string(); 

    /* 未启用视频网关功能 */
    // if (!gateway_enabled) {
    //     delete config;
    //     exit(0);
    // }

    // printf("[dss7016_server_ip]:   %s\n", dss7016_server_ip);
    // printf("[dss7016_server_port]: %d\n", dss7016_server_port);
    // printf("[dss7016_username]:    %s\n", dss7016_password);
    // printf("[dss7016_password]:    %s\n", dss7016_username);
    // printf("[things_server_ip]:    %s\n", things_server_ip);
    // printf("[things_server_port]:  %d\n", things_server_port);
    // printf("[gateway_device_tid]:  %s\n", gateway_device_tid);
    
    strcpy(DSS7016_LAN_IP,          dss7016_lan_ip);
    strcpy(DSS7016_WAN_IP,          dss7016_wan_ip);
    strcpy(DSS7016_SERVER_USERNAME, dss7016_username);
    strcpy(DSS7016_SERVER_PASSWORD, dss7016_password);
    DSS7016_SERVER_PORT           = dss7016_server_port;
        
    strcpy(SELF_TID, gateway_device_tid);
    sprintf(THINGS_SERVER, "host:%s;port:%d", things_server_ip, things_server_port);

    delete config;

    return 0;
}
