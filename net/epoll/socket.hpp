#pragma once

#include "epoll_fd_handler.hpp"
#include "epoll.hpp"
#include "logger.hpp"
#include <list>
#include <unordered_map>

namespace chr {

enum class protocol_t {
    unknown = 0,
    tcp,
    udp,
};

typedef epoll_handler_it* socket_key;
class raw_data_block_t final
{
public:
    raw_data_block_t(const byte_t* data, ssize_t len, socket_key key) {
        if (len > 0 && data) {
            m_len = len;
            m_key = key;
            m_buf = new byte_t[m_len];
            SPDLOG(DEBUG, "raw_data_block_t: {} bytes create!!!, this: {:p}", m_len, (void*)(this));
            memcpy(m_buf, data, m_len*sizeof(byte_t));
        }
    }
    ~raw_data_block_t() {
        SPDLOG(DEBUG, "raw_data_block_t: {} bytes destroy, this: {:p}", m_len, (void*)(this));
        delete [] m_buf;
    }
    raw_data_block_t(const raw_data_block_t& other) {
        delete [] m_buf;
        m_len = other.m_len;
        m_key = other.m_key;
        if (m_len) {
            SPDLOG(DEBUG, "raw_data_block_t: {} bytes copy (construct)!!!, this: {:p}", m_len, (void*)(this));
            m_buf = new byte_t[m_len];
            memcpy(m_buf, other.m_buf, m_len*sizeof(byte_t));
        }
    }
    raw_data_block_t& operator=(const raw_data_block_t& other) {
        delete [] m_buf;
        m_len = other.m_len;
        m_key = other.m_key;
        if (m_len) {
            SPDLOG(DEBUG, "raw_data_block_t: {} bytes copy (=)!!!, this: {:p}", m_len, (void*)(this));
            m_buf = new byte_t[m_len];
            memcpy(m_buf, other.m_buf, m_len*sizeof(byte_t));
        }
        return *this;
    }
    raw_data_block_t(raw_data_block_t&& other) {
        std::swap(m_len, other.m_len);
        std::swap(m_buf, other.m_buf);
        std::swap(m_key, other.m_key);
        SPDLOG(DEBUG, "raw_data_block_t: {} bytes wap, this: {:p}", m_len, (void*)(this));
    }
public:
    byte_t* m_buf = nullptr;
    ssize_t m_len = 0;
    socket_key m_key = nullptr;
};

typedef std::shared_ptr<raw_data_block_t> raw_data_block_sptr_t;
typedef std::list<raw_data_block_sptr_t>  raw_data_list_t;

class socket_t: public epoll_handler_it
{
    typedef struct {
        std::string local_addr = "";
        std::string remote_addr = "";
        uint16_t    local_port = 0;
        uint16_t    remote_port = 0;
    } peer_info_t;
public: 
    socket_t(protocol_t protocol, int sock_flags, poll_sptr_t poller, epoll_handler_sink_it* sink);
    socket_t(int fd, poll_sptr_t poller, epoll_handler_sink_it* sink, protocol_t protocol);
    virtual ~socket_t();
    int get_fd();
    ssize_t on_read();
    ssize_t on_write();
    int on_close();
    static int make_socket_non_blocking(int fd);
    void update_peer_info();
    const peer_info_t& peer_info() {
        return m_peer_info;
    }
    void set_is_listener(bool is_listener) {
        m_is_listener = is_listener;
    }
    bool is_listener() {
        return m_is_listener;
    }
    
    ssize_t write(const byte_t* buf, ssize_t length);

private:
    void    after_create();
    bool    has_write_pending() {
        return !m_write_pending_list.empty();
    }

private:
    protocol_t             m_protocol = protocol_t::unknown;
    int                    m_fd       = 0;
    poll_sptr_t            m_poller   = nullptr;
    epoll_handler_sink_it* m_sink     = nullptr;
    peer_info_t            m_peer_info;
    bool                   m_is_listener = false;
    raw_data_list_t        m_write_pending_list;
};

//typedef std::shared_ptr<chr::socket_t> socket_sptr_t;
typedef std::unique_ptr<chr::socket_t> socket_uptr_t;
typedef std::unordered_map<socket_key, socket_uptr_t> socket_map_t;

}