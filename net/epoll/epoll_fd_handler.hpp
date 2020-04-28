#pragma once

#include <sys/types.h>

namespace chr {

typedef unsigned char byte_t;

class epoll_handler_it
{
public:
    virtual ~epoll_handler_it(){}
    virtual int get_fd() = 0;
    virtual ssize_t on_read() = 0;
    virtual ssize_t on_write() = 0;
    virtual int on_close() = 0;
};

class epoll_handler_sink_it
{
public:
    virtual void on_new_connection() {}
    virtual void on_new_data(epoll_handler_it *which, byte_t* data, ssize_t count) = 0;
    virtual void on_closed(epoll_handler_it *which) = 0;
};

}