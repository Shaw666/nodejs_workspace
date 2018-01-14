#ifndef THINGS_H
#define THINGS_H

#include "common.h"
#include "cwlib.h"
#include "tobject.h"
//#undef emit
class Connection;
class VSync;

class Things {
 private:
    bool  _auto_login;
    char *_username;
    char *_password;
    bool _login_with_tid;
    int  _login_retry_cnt;

    int _seqnum;

    TObject *_vars;
    VSync *_sync_var;

    REQUEST      req_queue[REQUEST_QUEUE_MAX_LEN];
    SOCKET_STATE state_socket;
    Connection   *parent;


    char *in_buf;
    size_t  in_buf_len;
    size_t  in_buf_end;
    size_t  in_buf_last_STX;
    size_t  in_package_len;

    uint8_t   req_cnt;

    uint8_t timer_connect;         // maybe need only one.
    uint8_t timer_login;
    uint8_t timer_reconnect_interval;
    uint8_t timer_wait_aloha;
    uint8_t timer_vars_push;

    uint8_t cnt_keep_alive_retry;
    uint8_t timer_do_keep_alive;
    uint8_t timer_keep_alive_timeout;
  
    Event event_queue[EVENT_QUEUE_MAX_LEN];
    uint8_t event_q_head;
    uint8_t event_q_tail;

    STREAM_STATE state_stream;

    size_t  mtu;

    REQUEST_HANDLE *request_handles;
    int request_handle_cnt;

    void *context_user_on_event;
    void *context_user_on_request;
    void *context_user_on_post;

    Things_event_callback user_on_event;
    Things_callback user_on_request;
    Things_callback user_on_post;

    void init(const char* TID, const char *PASSKEY, Connection *conn,
              const size_t len_in, const size_t MTU);

    int  need_release_buffer(const SOCKET_EVENT code);
    Event* get_next_event();

    REQUEST* req_get_by_id(uint16_t id);
    void req_drop(REQUEST* req);
    int req_get_id();

    void on_response(char *from, char *to, char *data);
    void on_post(char *from, char *to, char *data);
    void on_request(char *from, char *to, char *data);
    void on_cmd_aloha(Event *e);
    void on_cmd_keep_alive_reply(Event *e);
    void on_cmd_login_reply(Event *e);
    void on_cmd_data(Event *e);
    void on_authed(Event *e);
    void on_auth_fail(Event *e);
    void on_server_available(Event *e);
    void on_keep_alive_timeout(Event* e);
    void on_need_auto_reconnect(Event* e);

    void do_reconnect();
    void reset();

    void in_buf_remove_package_and_check_for_next_package();
    void process_input_buf();
    void get_data_from_connection();
    void check_timer(uint8_t* timer, SOCKET_EVENT e);
    void post_event(const SOCKET_EVENT code, char *data, const size_t len);

    void send_cmd_login_to_parent();
    void send_cmd_keep_alive_to_parent();

    void reset_keep_alive_timer();

 public:

    int idle;
    int  authed;
    char *tid;
    char *passkey;
    char *sid;
    char *addr;

    long counter_tx_byte;
    long counter_tx_package;
    long counter_tx_error;
    long counter_rx_byte;
    long counter_rx_package;
    long counter_rx_error;

    /** An example member function.
     *  More details about this function.
     */

    Things(const char *TID, const char *PASSKEY, Connection *conn,
           const size_t len_in, const size_t MTU);

    Things(Connection *conn,
           const size_t len_in,const size_t len_out,
           const size_t MTU);

    ~Things();

    int send_cmd_begin();
    int data_send(const char *data);
    int data_send(const char ch);
    int cmd_end();
    void emit_event(T_EVENT e, void *data);
    
    SOCKET_STATE state();
    void connect_to(const char *conn_str );
    void loop();
    void stop();
    void time_tick();
    void on(THINGS_EVENT event, Things_callback handle, void *context);
    void on(THINGS_EVENT event, Things_event_callback handle, void *context);
    void on_request_url(const char *path, 
                        Things_callback handle, void *context);

    void login(const char *username, const char *password);

    void cmd_begin(const char *cmd, const char *dst);

    void cmd_beginEx(const char *cmd, const char *from, const char *dst);

    void pushEx(const char *from, const char *to,
                const char *type, const char *msg);

    void push(const char *dst, const char *type,const char *msg);

    int  requestEx(const char *from,
                   const char *dst, const char *url,
                   Things_callback cb, void *context);

    int  request(const char *dst, const char *url,
                 Things_callback handle, void *context);

    void responseEx(const char *from,
                    const char *to, uint8_t req_id, int code,
                    const char *header, const char *body);

    void response(const char *to, uint8_t req_id, int code,
                  const char *header, const char *body);

    static void on_request_var_query(Things *this_t, void *data,
                                     void *context);

    static void on_request_var_set(Things *this_t, void *data,
                                   void *context);

    static void on_var_change(TObject *var, void *context);

    static void on_request_var_sync(Things *this_t,
                                    void *data, void *context);

    static void on_request_var(Things *this_t,
                               void *data, void *context);
    void push_vars(int delta);
    void set_vars(TObject *v);
    TObject *vars();
    TObject *vars(const char *path);
    void process_var_message(T_POST *post);
    VSync *sync_var();
    TObject *sync_var(const char *tid);
    TObject *sync_var(const char *tid, const char *path);
};

/** \example acw.cpp
 * This is an example of how to use the Test class.
 * More details about this example.
 */

#endif //THINGS_H
