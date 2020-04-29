#include "raw_tcp_client.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace chr {

raw_tcp_client_t::raw_tcp_client_t(const std::string& host, const std::string& port)
    : m_host(host)
    , m_port(port)
{
    SPDLOG(DEBUG, "{} created: {}:{}, this: {:p}", __FUNCTION__, m_host, m_port, (void*)(this));
    m_read_chan = channel_t<raw_data_block_sptr_t>::create(1024);
    m_write_chan = channel_t<raw_data_block_sptr_t>::create(1024);  // old data blocks may stay long in channel and will be deleted untill channel ringbuf rorate.
    m_conn_result_chan = channel_t<int>::create();
}

raw_tcp_client_t::~raw_tcp_client_t()
{    
    SPDLOG(DEBUG, "{} destroy: {}:{}, this: {:p}", __FUNCTION__, m_host, m_port, (void*)(this));
}

int raw_tcp_client_t::connect()
{
    SPDLOG(DEBUG, "{} try connect to {}:{}", __FUNCTION__, m_host, m_port);    
    m_socket = socket_uptr_t(new socket_t(protocol_t::tcp, 0, ENGIN.get_epoll(), this));
    if (m_socket->get_fd() <= 0) {
        SPDLOG(ERROR, "{} connect to {}:{} failed, m_socket fd={} is invalid", __FUNCTION__, m_host, m_port, m_socket->get_fd());
        return -1;
    }
    int ret;
    struct sockaddr_in s_addr;
    memset(&s_addr, 0, sizeof (s_addr));
    s_addr.sin_family = AF_UNSPEC;
    s_addr.sin_port = htons(std::stoi(m_port));
    s_addr.sin_addr.s_addr = inet_addr(m_host.c_str());
    ret = ::connect(m_socket->get_fd(), (struct sockaddr*)&s_addr, sizeof (struct sockaddr));
    if (ret < 0) {
        if (errno == EINPROGRESS) {
            m_conn_result_chan->reset();
            m_socket->set_conn_res_chan(m_conn_result_chan);
            m_state = client_state_t::connecting;
            SPDLOG(DEBUG, "{} connect to {}:{} waiting for connection tobe done", __FUNCTION__, m_host, m_port);

            int conn_result = -1;    // -2 timeout, -1 failure, 0 success
            ENGIN.create_son_chroutine([&](void *) {
                SLEEP(15000);    // timeout in 15 seconds
                (*m_conn_result_chan) << -2;
            }, nullptr);
            (*m_conn_result_chan) >> conn_result;

            if (conn_result != 0) {
                SPDLOG(ERROR, "{} connect to {}:{} failed, conn_result={}", __FUNCTION__, m_host, m_port, conn_result);
            } else {
                socklen_t len;
                len = sizeof(ret);
                if (getsockopt(m_socket->get_fd(), SOL_SOCKET, SO_ERROR, &ret, &len) < 0) {
                    ret = -1;
                    SPDLOG(ERROR, "{} connect to {}:{} getsockopt failed, errno={}", __FUNCTION__, m_host, m_port, errno);
                }
            }
        } else {
            SPDLOG(ERROR, "{} connect to {}:{} failed, errno={}", __FUNCTION__, m_host, m_port, errno);
        }
    }

    if (ret == 0) {
        SPDLOG(DEBUG, "{} connect to {}:{} success!", __FUNCTION__, m_host, m_port);
        m_state = client_state_t::connected;   
        m_socket->update_peer_info();
    } else {
        m_state = client_state_t::disconnected;
        m_socket.reset();
    }
    return ret;
}

void raw_tcp_client_t::on_new_data(epoll_handler_it *which, byte_t* data, ssize_t count)
{
    if (data == nullptr || count == 0) {
        return;
    }
    (*m_read_chan) << raw_data_block_sptr_t(new raw_data_block_t(data, count, which));
}

void raw_tcp_client_t::read(raw_data_block_sptr_t &output)
{
    (*m_read_chan) >> output;
}

void raw_tcp_client_t::write(byte_t* data, ssize_t len)
{
    if (data == nullptr || len == 0) {
        return;
    }
    (*m_write_chan) << raw_data_block_sptr_t(new raw_data_block_t(data, len, nullptr));
}

int raw_tcp_client_t::select(int wait_ms)
{
    raw_data_block_sptr_t data_block = nullptr;
    int load = 0;
    while (m_state == client_state_t::connected && m_write_chan->read(&data_block, true)) {
        load++;
        if (data_block && m_socket) {
            SPDLOG(INFO, "{}: try write data to {}:{}"
                , __FUNCTION__
                , m_socket->peer_info().remote_addr
                , m_socket->peer_info().remote_port);
            m_socket->write(data_block->m_buf, data_block->m_len);
        }
    }
    return load;
}

void raw_tcp_client_t::on_closed(epoll_handler_it *which)
{
    if (m_socket) {
        SPDLOG(INFO, "{}: {}:{} closed"
            , __FUNCTION__
            , m_socket->peer_info().remote_addr
            , m_socket->peer_info().remote_port);
        m_state = client_state_t::disconnected;
        m_socket.reset();
    }
}

bool raw_tcp_client_t::is_connected()
{
    return m_state == client_state_t::connected;
}

}