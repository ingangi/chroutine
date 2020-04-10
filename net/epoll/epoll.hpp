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
#include "engine.hpp"

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

class epoll_t : public poll_it
{
    friend class engine_t;
    typedef std::map<int, epoll_handler_it*> fd_map_t;
    
public:
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