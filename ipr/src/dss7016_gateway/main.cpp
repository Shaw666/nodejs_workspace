#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "things/tobject.h"
#include "things/things.h"
#include "things/cwlib.h"
#include "things/conn_linux_socket.h"
#include "things/tobject.h"
#include "things/tparser.h"

#include "jnode_env.h"
#include "http_util.h"
#include "config.h"
#include "device.h"
#include "log.h"

#define T_DEBUG 1

#define TRUE             1
#define FALSE            0

#define MAX_BUF_IN       1024 * 1024 * 2
#define MAX_BUF_OUT      200000
#define MTU              1000

static Things *t             = NULL;
int things_followed_updating = 0;

void on_things_event(Things *this_t, T_EVENT event, void *data, void *context){
    VAR_EVENT *ve = (VAR_EVENT*)data;
    switch (event) {
    case E_SERVER_SERVICE_NOT_AVAILABLE:
    case E_SERVER_VERSION_NOT_SUPPORT:
    case E_SERVER_RESPONSE_ERROR:
        LOG_ERROR("Invalid Things Server");
        printf("Invalid server\n");
        break;
    case E_SERVER_NOT_READY:
        LOG_ERROR("Things Server Not Ready");
        printf("server not ready\n");
        break;
    case E_CONNECTED :
        LOG_INFO("Things Server Connected");
        printf("connected\n");
        break;
    case E_CONNECT_FAIL :
        LOG_ERROR("Things Server Connect Fail");
        printf("connect fail\n");
        break;
    case E_AUTHED :
        LOG_INFO("Things Server Authed");
        printf("authed\n");
        jnode_set_flag("things-server-connected", true);
        break;
    case E_CONNECTING:
        printf("connecting\n");
        break;
    case E_DISCONNECTED:
        LOG_INFO("Things Server Disconnected");
        printf("disconnected\n");
        jnode_set_flag("things-server-connected", false);
        d->things_server_connected = false;
        d->all_devices_logout();
        break;
    case E_RESPAWN:
        printf("respawn\n");
        break;
    case E_RESPAWNED:
        printf("respawned\n");
        break;
    case E_NEED_RECONNECT:
        printf("need reconnect\n");
        break;
    case E_AUTH_FAIL:
        LOG_INFO("Things Auth Fail");
        printf("auth fail\n");
        break;
    case E_FOLLOWED_NEW:  
        if (!things_followed_updating) {
            printf("\nfollowed new : tid = %s  path = %s\n", ve->tid, ve->path);
        };
        break;
    case E_FOLLOWED_RESET:
        printf("\nfollowed reset : tid = %s  path = %s\n", ve->tid, ve->path);
        break;
    case E_FOLLOWED_BEFORE_UPDATE:
    case E_FOLLOWED_AFTER_UPDATE:
        printf("\nfollowed update : tid = %s  path = %s\n", ve->tid, ve->path);
        break;
    case E_FOLLOWED_LOST_SYNC:
        printf("\nfollowed lost sync : tid = %s  path = %s\n", ve->tid, ve->path);
        break;
    case E_FOLLOWED_BEFORE_REMOVE:
    case E_FOLLOWED_AFTER_REMOVE:
        printf("\nfollowed remove : tid = %s  path = %s\n", ve->tid, ve->path);
        break;
    case E_FOLLOWED_BEFORE_START:
    case E_FOLLOWED_START:
        system("date");
        things_followed_updating = 1;        
        printf("\nfollowed start: count = %d\n", *(int*)data);
        break;
    case E_FOLLOWED_END:
        things_followed_updating = 0;
        system("date");
        printf("\nfollowed end\n");
        d->things_server_connected = true;
        break;
    case E_SERVER_TIME:
        printf("server time: %s\n", (char*)data);
        break;
    };
};

void signal_handler( int sig) {
    if(sig == SIGINT) {
        d->set_online_device_num(0);

        jnode_set_flag("things-server-connected", false);
        jnode_set_flag("dss7016-server-connected", false);

        if (t) {
            t->stop();
            delete t;
        }

        if (d) {
            delete d;
        }

        log_close();

        exit(-1);
    }
}

int main(int argc, char *argv[])
{
    int dss_connected = -1;
    
    signal(SIGINT, signal_handler);

    if (log_open("gateway") < 0) {
        printf("log open error!\n");
    }

    int res = config_init();
    if (res < 0) {
        LOG_ERROR("config init error, error code: %d", res);
        exit(-1);
    }

    /* Connect Things Server */
    Linux_socket *socket = new Linux_socket();
    t = new Things(SELF_TID, "", socket, MAX_BUF_IN, MTU);
    t->on(ON_EVENT,   on_things_event, NULL);
    t->connect_to(THINGS_SERVER);

    d = new Device(t);
    if (0 == d->init()) {
        jnode_set_flag("dss7016-server-connected", true);
        printf("Device init OK\n");
    }
    
    int tick_cnt = 0;
    time_t t_last, t_now;
    time(&t_last);
    while (1) {
		time(&t_now);		
		if (t_now - t_last >= 1){
			t_last = t_now;			

            // if ((++tick_cnt % 10) == 0) {
            //     printf("\n");
            // } else {
            //     printf(".");
            //     fflush(stdout);
            // }

            t->time_tick();
            d->tick_1s();
		}

        if (t) {
            t->loop();
        }

        if (d) {
            d->loop();
        }

        if (t->idle) {
            usleep(200 * 1000);
        } else {
            usleep(10);
        }
    }

    if (t) {
        delete t;
    }

    if (d) {
        delete d;
    }

    log_close();
    
	return 0;
}
