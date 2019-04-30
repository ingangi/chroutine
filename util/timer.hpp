/// \file timer.hpp
/// 
/// timer used in chroutine
///
/// \author ingangi
/// \version 0.1.0
/// \date 2019-04-30

#ifndef TIMER_HPP
#define TIMER_HPP

#include <functional>
#include "selectable_obj.hpp"
#include "channel.hpp"

namespace chr {

typedef std::function<void(void *)> timer_callback_t;

// not thread safe!
// should be used in the same chroutine_thread_t
class chr_timer_t : public selectable_object_it
{
public:
    chr_timer_t();
    ~chr_timer_t();
    virtual int select(int wait_ms);

    bool start(uint32_t interval_ms, const timer_callback_t& cb);
    void stop();

private:
    bool                m_running = false;
    timer_callback_t    m_cb = nullptr;
    channel_sptr_t      m_trigger = nullptr;
    chan_selecter_t     m_selecter;
    chroutine_id_t      m_trigger_chroutine_id = INVALID_ID;
};

}

#endif