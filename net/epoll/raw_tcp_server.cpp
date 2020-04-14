#include "raw_tcp_server.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
namespace chr {

raw_tcp_server_t::raw_tcp_server_t(const std::string& host, const std::string& port)
    : m_host(host)
    , m_port(port)
{
    SPDLOG(DEBUG, "{} created: {}:{}, this: {:p}", __FUNCTION__, m_host, m_port, (void*)(this));
    m_read_chan = channel_t<raw_data_block_sptr_t>::create(1024);
    m_write_chan = channel_t<raw_data_block_sptr_t>::create(1024);  // old data blocks may stay long in channel and will be deleted untill channel ringbuf rorate.
}

raw_tcp_server_t::~raw_tcp_server_t()
{    
    SPDLOG(DEBUG, "{} destroy: {}:{}, this: {:p}", __FUNCTION__, m_host, m_port, (void*)(this));
}

void raw_tcp_server_t::start()
{
    SPDLOG(DEBUG, "{} starting on: {}:{}", __FUNCTION__, m_host, m_port);
    m_state = server_state_t::starting;

    struct addrinfo hints;
    struct addrinfo *result, *result_iter;
    int error;

    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // 1, for bind
    error = getaddrinfo(m_host.c_str(), m_port.c_str(), &hints, &result);
    if (error != 0) {
        SPDLOG(ERROR, "{}: getaddrinfo error {}", __FUNCTION__, strerror(errno));
        return;
    }

    int opt = SO_REUSEADDR;
    for (result_iter = result; result_iter != nullptr; result_iter = result_iter->ai_next) {
        m_listener = socket_uptr_t(new socket_t(protocol_t::tcp, result_iter->ai_protocol, ENGIN.get_epoll(), this));
        if (m_listener->get_fd() <= 0)
            continue;

        if (setsockopt(m_listener->get_fd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1) {
            SPDLOG(ERROR, "{}: setsockopt error {}", __FUNCTION__, strerror(errno));
            continue;
        }
        error = bind(m_listener->get_fd(), result_iter->ai_addr, result_iter->ai_addrlen);
        if (error == 0)
            break;
    }
    
    if (result_iter == nullptr) {
        SPDLOG(ERROR, "{}: bind failed");
        assert(0);
        return;
    }

    freeaddrinfo(result);
    error = listen (m_listener->get_fd(), SOMAXCONN);
    if (error == -1) {
        SPDLOG(ERROR, "{}: listen error {}", __FUNCTION__, strerror(errno));
        return;
    }

    m_listener->set_is_listener(true);
    m_listener->update_peer_info();
    m_state = server_state_t::serving;
}

void raw_tcp_server_t::on_new_data(epoll_handler_it *which, byte_t* data, ssize_t count)
{
    auto iter = m_connections.find(which);
    if (iter == m_connections.end()) {
        SPDLOG(ERROR, "can't find connection, drop data, len={}", count);
        return;
    }
    if (data == nullptr || count == 0) {
        return;
    }
    // raw server has no buf for data
    (*m_read_chan) << raw_data_block_sptr_t(new raw_data_block_t(data, count, which)); // will block if channel is full
}

void raw_tcp_server_t::read(raw_data_block_sptr_t &output)
{
    (*m_read_chan) >> output;
}

void raw_tcp_server_t::write(const socket_key& which, byte_t* data, ssize_t len)
{
    if (which == nullptr || data == nullptr || len == 0) {
        return;
    }
    (*m_write_chan) << raw_data_block_sptr_t(new raw_data_block_t(data, len, which));
}

int raw_tcp_server_t::select(int wait_ms)
{
    raw_data_block_sptr_t data_block = nullptr;
    int load = 0;
    while (m_write_chan->read(&data_block, true)) {
        load++;
        if (data_block) {
            auto iter = m_connections.find(data_block->m_key);
            if (iter != m_connections.end()) {
                auto &sock = iter->second;
                if (sock) {
                    SPDLOG(INFO, "{}: try write data to {}:{}"
                        , __FUNCTION__
                        , sock->peer_info().remote_addr
                        , sock->peer_info().remote_port);
                    sock->write(data_block->m_buf, data_block->m_len);
                }
            }
        }
    }
    return load;
}

void raw_tcp_server_t::on_new_connection()
{
    while (1) {
        struct sockaddr in_addr;
        socklen_t in_len;
        int infd;
        in_len = sizeof(in_addr);
        infd = accept(m_listener->get_fd(), &in_addr, &in_len);
        if (infd <= 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                SPDLOG(ERROR, "{}: accept error {}", __FUNCTION__, strerror(errno));
            }
            break;
        } else {
            auto new_sock = socket_uptr_t(new socket_t(infd, ENGIN.get_epoll(), this, protocol_t::tcp));
            socket_key key = static_cast<socket_key>(new_sock.get());
            if (key) {
                new_sock->update_peer_info();
                SPDLOG(INFO, "{}: new connection established: {}:{} <==> {}:{}"
                    , __FUNCTION__
                    , new_sock->peer_info().local_addr
                    , new_sock->peer_info().local_port
                    , new_sock->peer_info().remote_addr
                    , new_sock->peer_info().remote_port);
                m_connections[key] = std::move(new_sock);
            }
        }
    }    
}

}