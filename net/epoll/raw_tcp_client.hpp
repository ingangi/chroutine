#pragma once

#include "epoll_fd_handler.hpp"
#include "socket.hpp"

namespace chr {


class raw_tcp_client_t: public epoll_handler_sink_it, public selectable_object_it
{
    enum class client_state_t {
        creating = 0,
        connecting,
        connected,
        disconnected,
    };
public:
    static chr::selectable_object_sptr_t create(const std::string& host, const std::string& port) {
        const std::thread::id & epoll_thread = ENGIN.epoll_thread_id();
        assert(epoll_thread != NULL_THREAD_ID);
        raw_tcp_client_t *p_this = new raw_tcp_client_t(host, port);
        chr::selectable_object_sptr_t s_this = p_this->register_to_engin(epoll_thread);		
        if (s_this.get() == nullptr) {
            delete p_this;
        } 
        return s_this;
    }
    virtual ~raw_tcp_client_t();
    virtual int connect();
    virtual void read(raw_data_block_sptr_t &output);
    virtual void write(byte_t* data, ssize_t len);
    virtual void on_new_data(epoll_handler_it *which, byte_t* data, ssize_t count);
    virtual void on_closed(epoll_handler_it *which);
    virtual int select(int wait_ms);
    bool is_connected();

private:
    raw_tcp_client_t(const std::string& host, const std::string& port);

private:
    socket_uptr_t  m_socket = nullptr;
    client_state_t m_state  = client_state_t::creating;
    std::string    m_host;
    std::string    m_port;
    raw_data_chan_t m_read_chan;
    raw_data_chan_t m_write_chan;
    socket_conn_res_chan_t  m_conn_result_chan;
};

}