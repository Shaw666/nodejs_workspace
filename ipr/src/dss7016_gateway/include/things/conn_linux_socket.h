#ifndef CONN_LINUX_SOCKET_H
#define CONN_LINUX_SOCKET_H
#include <netinet/in.h>
#include "connection.h"


class Things;
class Linux_socket: public Connection {
 private:
  char     _host[50];
  uint16_t _port;

  char *_tx_buf;
  ssize_t _tx_buf_len;

  struct sockaddr_in serv_addr;

  int do_send_tx_buf_out();
  
 public:
  int sockfd;

  Linux_socket();
  ~Linux_socket();
  virtual int set_connect_info(const char* connection_str);
  virtual int do_connect_to();
  virtual int do_disconnect();
  virtual int connected(); //0 = not connected  1 = connected ; -1 = error
  virtual ssize_t do_send_data(const char* buf, size_t len);
  virtual ssize_t do_recv_data(char* buf, size_t len);
};

#endif

