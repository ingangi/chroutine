#pragma once

#include "epoll_fd_handler.hpp"
#include "socket.hpp"
#include "channel.hpp"
#include <algorithm>    // std::swap

namespace chr {

enum class server_state_t {
    creating = 0,
    starting,
    serving,
};

typedef std::shared_ptr<channel_t<raw_data_block_sptr_t> > raw_data_chan_t;

class raw_tcp_server_t: public epoll_handler_sink_it, public selectable_object_it
{
public:
    static chr::selectable_object_sptr_t create(const std::string& host, const std::string& port) {
        const std::thread::id & epoll_thread = ENGIN.epoll_thread_id();
        assert(epoll_thread != NULL_THREAD_ID);
        raw_tcp_server_t *p_this = new raw_tcp_server_t(host, port);
        chr::selectable_object_sptr_t s_this = p_this->register_to_engin(epoll_thread);		
        if (s_this.get() == nullptr) {
            delete p_this;
        } 
        return s_this;
    }

    virtual ~raw_tcp_server_t();
    virtual void start();
    virtual void read(raw_data_block_sptr_t &output);
    virtual void write(const socket_key& which, byte_t* data, ssize_t len);

    virtual void on_new_data(epoll_handler_it *which, byte_t* data, ssize_t count);
    virtual void on_new_connection();
    
    virtual int select(int wait_ms);

private:
    raw_tcp_server_t(const std::string& host, const std::string& port);

private:
    socket_uptr_t  m_listener = nullptr;
    server_state_t m_state    = server_state_t::creating;
    socket_map_t   m_connections;
    std::string    m_host;
    std::string    m_port;
    raw_data_chan_t m_read_chan;
    raw_data_chan_t m_write_chan;
    // chan_selecter_t m_write_selecter;
    // raw_data_block_sptr_t m_data_block_for_write = nullptr;
};

}