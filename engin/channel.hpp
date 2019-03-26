/// \file channel.hpp
/// 
/// channel_t is for the communication between chroutines
///
/// \author ingangi
/// \version 0.1.0
/// \date 2019-03-26

#ifndef CHANNEL_H
#define CHANNEL_H

#include <deque>
#include "chroutine.hpp"

template<typename T>
class channel_t
{
    typedef std::shared_ptr<channel_t<T> > channel_sptr_t;
public:
    virtual ~channel_t(){}
    static channel_sptr_t create() {
        return channel_sptr_t(new channel_t<T>);
    }

private:
    channel_t(){}

private:
    T                           m_data;
    std::deque<chroutine_pos_t> m_waiting_que;
    bool                        m_closed = false;
    size_t                      m_send_index = 0;
    size_t                      m_recv_index = 0;
};


#endif