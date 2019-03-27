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
    typedef struct
    {
        std::thread::id thread_id;
        chroutine_id_t  chrotine_id = INVALID_ID;
        T *             out_ptr = nullptr;
        T               in_data;
    }chroutine_chan_context_t;

    typedef std::shared_ptr<channel_t<T> > channel_sptr_t;
public:
    virtual ~channel_t(){
        delete [] m_data_array;
    }
    static channel_sptr_t create(int max_size = 1) {
        return channel_sptr_t(new channel_t<T>(max_size));
    }

    // write to the channel
    void operator << (const T& data) {
        if (writable() < 1) {
            // add to m_waiting_write_que
            chroutine_chan_context_t ctx;
            ctx.thread_id = std::this_thread::get_id();
            ctx.chrotine_id = ENGIN.get_current_chroutine_id();
            ctx.in_data = data;
            m_waiting_write_que.push_back(ctx);
            // block the chroutine
            block();
            return;
        }

        // if some one is waiting, give him the data directly
        if (!m_waiting_read_que.empty()) {
            chroutine_chan_context_t &context = m_waiting_read_que.front();
            *(context.out_ptr) = data;
            // wake the chroutine
            ENGIN.awake_chroutine(context.thread_id, context.chrotine_id);
            m_waiting_read_que.pop_front();
            return;
        }

        m_data_array[m_w_index] = data;
        m_unread++;
        m_w_index = (m_w_index + 1) % m_max_size;
    }

    // read from the channel
    void operator >> (T& data) {
        if (readable() < 1) {
            // add to m_waiting_read_que
            chroutine_chan_context_t ctx;
            ctx.thread_id = std::this_thread::get_id();
            ctx.chrotine_id = ENGIN.get_current_chroutine_id();
            ctx.out_ptr = &data;
            m_waiting_read_que.push_back(ctx);
            // block the chroutine
            block();
            return;
        }

        data = m_data_array[m_r_index];
        m_unread--;
        m_r_index = (m_r_index + 1) % m_max_size;

        // if some one is waiting for write
        if (!m_waiting_write_que.empty()) {
            chroutine_chan_context_t &context = m_waiting_write_que.front();
            *this <<  context.in_data;
            // wake the chroutine
            ENGIN.awake_chroutine(context.thread_id, context.chrotine_id);
            m_waiting_write_que.pop_front();
        }
    }    

private:
    channel_t(int max_size) : m_max_size(max_size) {
        if (m_max_size <= 0)
            m_max_size = 1;
        m_data_array = new T[m_max_size];
    }

    int writable() {
        // return m_max_size - m_w_index;
        return m_max_size - m_unread;
    }

    int readable() {
        // return m_w_index - m_r_index;
        return m_unread;
    }

    void block() {
        SLEEP(518400000);
    }


private:
    bool    m_closed = false;
    int     m_w_index = 0;
    int     m_r_index = 0;
    int     m_max_size = 1;
    T *     m_data_array = nullptr;
    int     m_unread = 0;
    
    std::deque<chroutine_chan_context_t> m_waiting_write_que;
    std::deque<chroutine_chan_context_t> m_waiting_read_que;
};


#endif