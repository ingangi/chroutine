#include <sys/epoll.h>
#include <errno.h>
#include "epoll.hpp"
#include "handler.hpp"

namespace chr {
epoll_t::epoll_t()
{    
    m_epoll_fd = epoll_create1(0);
    if (m_epoll_fd < 0) {
        SPDLOG(ERROR, "epoll_t epoll_create failed: {}", strerror(errno));
    }
}

epoll_t::~epoll_t()
{
    ::close(m_epoll_fd);
}

int epoll_t::add_fd(epoll_handler_it* handler, int32_t flag)
{
    if (!handler) {
        return 0;
    }

    int fd = handler->get_fd();
    if (fd <= 0) {
        return fd;
    }

    std::pair<fd_map_t::iterator, bool> ret = m_fd_map.insert(std::pair<int, epoll_handler_it*>(fd, handler));
    if (ret.second) {
    } else {
        del_fd(ret.first->second);
        m_fd_map.insert(std::pair<int, epoll_handler_it*>(fd, handler));
    }

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events |= flag;
    ev.data.fd = fd;
    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        SPDLOG(ERROR, "epoll_ctl(EPOLL_CTL_ADD) failed: {}", strerror(errno));
        close_fd(handler);
        return 0;
    }

    SPDLOG(INFO, "epoll_t::add_fd {}, ptr:{:p}", fd, (void*)handler);
    return fd;
}

int epoll_t::mod_fd(int fd, int32_t flag)
{
    epoll_handler_it* handler = get_handler(fd);
    if (handler == nullptr) {
        SPDLOG(INFO, "{} error: fd {} not exist.", __FUNCTION__, fd);
        return -1;
    }
    
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events |= flag;
    ev.data.fd = fd;
    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &ev) < 0) {
        SPDLOG(ERROR, "epoll_ctl(EPOLL_CTL_MOD) failed: {}", strerror(errno));
        close_fd(handler);
        return -1;
    }

    return 0;
}

int epoll_t::del_fd(epoll_handler_it* handler)
{
    if (!handler || handler->get_fd() == 0) {
        return 0;
    }

    int fd = handler->get_fd();
    m_fd_map.erase(fd);
    
    epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    SPDLOG(INFO, "epoll_t::del_fd {}, ptr:{:p}", fd, (void*)handler);

    return fd;
}

int epoll_t::close_fd(int fd)
{
    return close_fd(get_handler(fd));
}

int epoll_t::close_fd(epoll_handler_it* handler)
{
    if (!handler) {
        return 0;
    }
    handler->on_close();
    return del_fd(handler);
}

epoll_handler_it* epoll_t::get_handler(int fd)
{
    fd_map_t::iterator iter = m_fd_map.find(fd);
    if (iter == m_fd_map.end()) {
        return nullptr;
    }
    return iter->second;
}

int epoll_t::select(int wait_ms)
{
    const int MAXEVENTS = 8;
    struct epoll_event events[MAXEVENTS];
    int n = epoll_wait(m_epoll_fd, events, MAXEVENTS, wait_ms);
    if (n > 0) {
        SPDLOG(DEBUG, "epoll_wait detect events count: {}", n);
    }

    for (int i = 0; i < n; i++) {
        if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
            SPDLOG(INFO, "epoll_t::pool epoll_wait failed! events: {}", events[i].events);
            close_fd(events[i].data.fd);
            continue;
        } 

        epoll_handler_it* handler = get_handler(events[i].data.fd);
        if (!handler) {
            SPDLOG(ERROR, "epoll_t::pool: can't find epoll_handler_it for fd: {}", events[i].data.fd);
            epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
            continue;
        }

        if (events[i].events & EPOLLIN) {
            while (1) {
                ssize_t count = handler->on_read();
                if (count == -1) {
                    if (errno != EAGAIN) {
                        SPDLOG(ERROR, "epoll_handler_it({}) read failed: {}", events[i].data.fd, strerror(errno));
                    }
                    break;
                } else if (count == 0) {
                    close_fd(handler);
                    break;
                }
            }
        }

        if (events[i].events & EPOLLOUT) {
            handler->on_write();
        }
    }
    return n;
}
}