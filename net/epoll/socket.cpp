#include "socket.hpp"
#include <cassert>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace chr {

int socket_t::make_socket_non_blocking(int fd)
{
    int flags, s;
    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        SPDLOG(ERROR, "fcntl:", strerror(errno));
        return -1;
    }
    flags |= O_NONBLOCK;
    s = fcntl (fd, F_SETFL, flags);
    if (s == -1) {
        SPDLOG(ERROR, "fcntl:", strerror(errno));
        return -1;
    }
    return 0;
}


socket_t::socket_t(protocol_t protocol, int sock_flags, poll_sptr_t poller, epoll_handler_sink_it* sink)
    : m_protocol(protocol)
    , m_poller(poller)
    , m_sink(sink)
{
    assert(m_poller != nullptr && m_sink != nullptr);
    assert(m_protocol == protocol_t::tcp);  //for now
    int sock_type = m_protocol == protocol_t::tcp ? SOCK_STREAM : SOCK_DGRAM;
    m_fd = socket(AF_INET/*AF_UNSPEC*/, sock_type, sock_flags);
    if (m_fd > 0) {
        after_create();
    }

    SPDLOG(DEBUG, "socket_t::socket_t() created, fd: {}, protocol: {}, [sock_type={}, sock_flags={}], this: {:p}"
        , m_fd
        , static_cast<int>(m_protocol)
        , sock_type
        , sock_flags
        , (void*)(this));
}

socket_t::socket_t(int fd, poll_sptr_t poller, epoll_handler_sink_it* sink, protocol_t protocol)
    : m_protocol(protocol)
    , m_fd(fd)
    , m_poller(poller)
    , m_sink(sink)
{
    assert(m_fd > 0);
    after_create();
    SPDLOG(DEBUG, "socket_t::socket_t() created, fd: {}, this: {:p}", m_fd, (void*)(this));
}

void socket_t::after_create()
{
    SPDLOG(DEBUG, "{}, m_fd = {}", __FUNCTION__, m_fd);
    assert(make_socket_non_blocking(m_fd) == 0);
    if (m_poller) {
        m_poller->add_fd(this, EPOLLIN | EPOLLET);
    }
}

socket_t::~socket_t()
{
    SPDLOG(DEBUG, "socket_t::~socket_t() destroy, fd: {}, this: {:p}", m_fd, (void*)(this));
    m_poller->close_fd(this);
}

int socket_t::get_fd()
{
    return m_fd;
}

ssize_t socket_t::on_read()
{
    if (is_listener()) {
        m_sink->on_new_connection();
        errno = EAGAIN;
        return -1; 
    }

    byte_t buf[1024] = { 0 };
    ssize_t count = read(m_fd, buf, sizeof(buf));
    SPDLOG(DEBUG, "socket_t::on_read() read count: {}, m_fd={}", count, m_fd);

    if (count > 0) {
        m_sink->on_new_data(this, buf, count);
    }
    return count;
}

ssize_t socket_t::on_write()
{
    m_poller->mod_fd(m_fd, EPOLLIN | EPOLLET);

    if (m_conn_res_chan != nullptr) {
        (*m_conn_res_chan) << 0;
        m_conn_res_chan = nullptr;
    }

    ssize_t total_written = 0;
    bool abort = false;
    while (has_write_pending() && !abort) {
        auto &data = m_write_pending_list.front();
        if (data && data->m_buf && data->m_len) {          
            ssize_t bytes_left = data->m_len; 
            const byte_t *ptr = data->m_buf;
            while (bytes_left > 0) {
                ssize_t written_bytes = ::write(m_fd, ptr, bytes_left); 
                SPDLOG(DEBUG, "socket_t::on_write() write count: {}, m_fd={}", written_bytes, m_fd);
                if(written_bytes<=0) {        
                    if (errno == EINTR) {
                        written_bytes = 0;
                    } else {
                        break;
                    }
                } 
                bytes_left -= written_bytes; 
                ptr += written_bytes;
                total_written += written_bytes;
            }
            if (bytes_left > 0) {
                SPDLOG(ERROR, "socket_t::on_write() write not finish, left length: {}, m_fd={}", bytes_left, m_fd);
                m_poller->mod_fd(m_fd, EPOLLIN | EPOLLOUT | EPOLLET);
                m_write_pending_list.push_front(raw_data_block_sptr_t(new raw_data_block_t(ptr, bytes_left, this)));
                abort = true;
            }
        }
        m_write_pending_list.pop_front();
    }
    SPDLOG(DEBUG, "socket_t::on_write(), m_fd={}, total_written {} bytes", m_fd, total_written);
    return total_written;   
}

int socket_t::on_close()
{    
    SPDLOG(INFO, "socket_t::on_close(), m_fd={}, remote: {}:{}"
        , m_fd
        , m_peer_info.remote_addr
        , m_peer_info.remote_port);
    m_sink->on_closed(this);
    return ::close(m_fd);
}

ssize_t socket_t::write(const byte_t* buf, ssize_t length)
{
    if (buf == nullptr || length == 0) {
        return 0;
    }
    if (has_write_pending()) {
        m_write_pending_list.push_back(raw_data_block_sptr_t(new raw_data_block_t(buf, length, this)));
        return 0;
    }

    ssize_t bytes_left = length; 
    const byte_t *ptr = buf;
    while (bytes_left > 0) {
        ssize_t written_bytes = ::write(m_fd, ptr, bytes_left); 
        SPDLOG(DEBUG, "socket_t::write() write count: {}, m_fd={}", written_bytes, m_fd);
        if(written_bytes<=0) {        
            if (errno == EINTR) {
                written_bytes = 0;
            } else {
                break;
            }
        } 
        bytes_left -= written_bytes; 
        ptr += written_bytes;
    } 

    if (bytes_left > 0) {
        SPDLOG(ERROR, "write not finish, left length: {}, m_fd={}", bytes_left, m_fd);
        if (!has_write_pending()) {
            m_poller->mod_fd(m_fd, EPOLLIN | EPOLLOUT | EPOLLET);
        }
        m_write_pending_list.push_back(raw_data_block_sptr_t(new raw_data_block_t(ptr, bytes_left, this)));  
    }
    return length - bytes_left;
}

void socket_t::update_peer_info()
{
    struct sockaddr_in local_addr, remote_addr;
    socklen_t addr_len = sizeof(local_addr);
    if (getsockname(m_fd, (struct sockaddr *)&local_addr, &addr_len) == 0) {
        m_peer_info.local_addr = inet_ntoa(local_addr.sin_addr);
        m_peer_info.local_port = ntohs(local_addr.sin_port);
    }

    if (m_protocol == protocol_t::tcp) {
        if (getpeername(m_fd, (struct sockaddr *)&remote_addr, &addr_len) == 0) {
            char ip_addr[INET_ADDRSTRLEN];
            m_peer_info.remote_addr = inet_ntop(AF_INET, &remote_addr.sin_addr, ip_addr, sizeof(ip_addr));
            m_peer_info.remote_port = ntohs(remote_addr.sin_port);
        }
    }
}

}