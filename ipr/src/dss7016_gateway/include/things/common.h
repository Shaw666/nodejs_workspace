#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#define THINGS_DEBUG 1
#define T_DEBUG 1


#define STX  2    //0x02
#define ETX  3    //0x03
#define GS   29   //0x1D
#define US   31   //0x1F

#define MAX_USERNAME_LEN         32
#define MAX_PASSWORD_LEN         32
#define MAX_TID_LEN              32
#define MAX_SID_LEN              32

#define TIME_CONNECT_TIMEOUT     15
#define TIME_RECONNECT_INTERVAL  5
#define TIME_LOGIN_TIMEOUT       10

#define TIME_KEEP_LIVE_INTERVAL  15
#define TIME_KEEP_LIVE_TIMEOUT   10
#define CNT_KEEP_LIVE_MAX_RETRY  3

#define REQUEST_QUEUE_MAX_LEN    10
#define REQUEST_TIMEOUT          10

#define CMD_PUSH             "p"
#define CMD_POST             "t"
#define CMD_REQUEST          "q"
#define CMD_RESPONSE         "r"
#define CMD_SUBSCRIBE        "s"
#define CMD_UNSUBSCRIBE      "u"
#define CMD_BROADCAST        "b"

#define CMD_ALOHA            "a"
#define CMD_LOGIN            "l"
#define CMD_LOGIN_REPLY      "o"
#define CMD_KEEP_ALIVE       "k"
#define CMD_KEEP_ALIVE_REPLY "r"
#define CMD_DATA             "d"

#define ADDR_SYS_ROOT        ".."
#define ADDR_APP_ROOT        "."

#define EVENT_QUEUE_MAX_LEN    10
#define TID_MAX_LEN            18
#define THINGS_ADDR_MAX_LEN    10
#define PUSH_VAR_INTERVAL      10

typedef enum {
    ON_EVENT = 1,
    ON_POST,
    ON_REQUEST
} THINGS_EVENT;

typedef enum {
    STREAM_STATE_WAIT_STX = 0,
    STREAM_STATE_WAIT_ETX
} STREAM_STATE;

typedef enum {
    SOCKET_IDLE = 0,
    SOCKET_CONNECTING,
    SOCKET_SERVER_NA,
    SOCKET_WAIT_RECONNECT,
    SOCKET_WAIT_ALOHA,
    SOCKET_CONNECTED,
    SOCKET_AUTHED,
    SOCKET_AUTHED_RESPAWN
} SOCKET_STATE;

typedef enum {
    E_CONNECTING = 0,
    E_FRAME_ERROR_UNEXCEPT_DATA,
    E_FRAME_ERROR_UNEXCEPT_STX,
    E_SERVER_SERVICE_NOT_AVAILABLE,
    E_SERVER_VERSION_NOT_SUPPORT,
    E_SERVER_RESPONSE_ERROR,
    E_SERVER_NOT_READY,
    E_CONNECTED,
    E_CONNECT_FAIL,
    E_DISCONNECTED,
    E_RESPAWN,
    E_RESPAWNED,
    E_NEED_RECONNECT,
    E_AUTHED,
    E_AUTH_FAIL,
    E_SERVER_TIME,
    E_FOLLOWED_BEFORE_START,
    E_FOLLOWED_START,
    E_FOLLOWED_END,
    E_FOLLOWED_NEW,
    E_FOLLOWED_RESET,
    E_FOLLOWED_BEFORE_UPDATE,
    E_FOLLOWED_AFTER_UPDATE,
    E_FOLLOWED_LOST_SYNC,
    E_FOLLOWED_BEFORE_REMOVE,
    E_FOLLOWED_AFTER_REMOVE,
} T_EVENT;

struct VAR_EVENT{
    char *tid;
    char *path;  // @todo expand compressed path
}; 

typedef enum {
    E_user_connect_to = 1,
    E_socket_connected,
    E_connect_timeout,
    E_aloha_timeout, 
    E_user_login,
    E_stream_data_in,
    E_auth_fail,
    E_authed,
    E_stream_data_out,
    E_login_reply_timeout,
    E_vars_push,

    E_cmd_aloha,
    E_cmd_data,
    E_cmd_login_reply,
    E_cmd_keep_alive_reply,

    E_login_max_retry,
    E_socket_close,
    E_socket_drain,
    E_socket_timeout,
    E_socket_error,
    E_socket_end,
    E_user_logout,
    E_user_disconnect,
    E_do_keep_alive,
    E_keep_alive_max_retry,
    E_keep_alive_timeout,
    E_server_available,
    E_server_not_available,
    E_reconnect_max_retry,
    E_reconnect_interval,
} SOCKET_EVENT;

class Things;

typedef struct _Event Event;
typedef struct _REQUEST REQUEST;

typedef void (*Things_event_handle)(Event* e);
typedef void (*Things_do_connect)();
typedef int (*Things_do_send_data)(char* data, size_t len);

typedef void (*Things_callback)(Things *this_t, void *data, void *context);
typedef void (*Things_event_callback)(Things *this_t, T_EVENT event, void *data, void *context);

struct _Event {
    SOCKET_EVENT code;
    char*        data;
    size_t       len;
};
struct _REQUEST {
    uint8_t req_id;
    uint8_t timeout;
    Things_callback callback;
    void *context;
};

struct REQUEST_HANDLE {
    char *path;
    Things_callback handle;
    void *context;
};

struct T_AUTH {
    int code;
};

struct T_REQUEST {
    char *from;
    char *to;
    int   req_id;
    char *url;
};

struct T_POST {
    char *from;
    char *to;
    char *readers;
    char *src;
    char *type;
    char *message;
};

struct T_RESPONSE {
    uint8_t req_id;
    int status_code;
    char* header;
    char* body;
};
#endif
