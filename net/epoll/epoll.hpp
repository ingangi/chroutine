#pragma once

/// \file epoll.hpp
/// 
/// 
///
/// \author ingangi
/// \version 0.1.0
/// \date 2020-04-01

#include <map>
#include <stdint.h>
#include "selectable_obj.hpp"

namespace chr {

class epoll_handler_it;
class poll_it : public selectable_object_it
{
public:
    virtual int add_fd(epoll_handler_it* handler, int32_t flag) = 0;
    virtual int del_fd(epoll_handler_it* handler) = 0;
    virtual int mod_fd(int fd, int32_t flag) = 0;
    virtual int close_fd(int fd) = 0;
    virtual int close_fd(epoll_handler_it* handler) = 0;
};

typedef std::shared_ptr<poll_it>  poll_sptr_t;

class epoll_t : public poll_it
{
    typedef std::map<int, epoll_handler_it*> fd_map_t;
    
public:
    static poll_sptr_t create();
    ~epoll_t();

    int add_fd(epoll_handler_it* handler, int32_t flag);
    int del_fd(epoll_handler_it* handler);
    int mod_fd(int fd, int32_t flag);
    int close_fd(int fd);
    int close_fd(epoll_handler_it* handler);
    
    int select(int wait_ms);

private:
    epoll_t();
    epoll_handler_it* get_handler(int fd);

private:
    fd_map_t    m_fd_map;
    int         m_epoll_fd;
};

}