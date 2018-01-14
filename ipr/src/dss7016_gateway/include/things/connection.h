#ifndef CONNECTION_H
#define CONNECTION_H

#ifdef AVR
#include <arduino.h>
#define ssize_t int
#else
#include <sys/types.h>
#endif

#include <stdlib.h>
#include <inttypes.h>


class Things;

class Connection {
 public:
    Things  *owner;
    Connection() { owner = 0;};
    virtual ~Connection() {};
    virtual int set_connect_info(const char* connection_str) = 0;
    virtual int do_connect_to() = 0;
    virtual int do_disconnect() = 0;
    virtual int connected() = 0;
    virtual ssize_t do_send_data(const char* buf, size_t len) = 0;
    virtual ssize_t do_recv_data(char* buf, size_t len) = 0;
};

#endif //CONECTION_H
