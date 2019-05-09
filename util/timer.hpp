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
#include "chan_selecter.hpp"

namespace chr {

typedef std::function<void()> timer_callback_t;

// not thread safe!
// should be used in the same chroutine_thread_t
class chr_timer_t : public selectable_object_it
{
public:
    ~chr_timer_t();
    virtual int select(int wait_ms);
        
    // create and register to engin
	static chr_timer_t* create(uint32_t interval_ms, timer_callback_t cb) {
		chr_timer_t *p_this = new chr_timer_t(interval_ms, cb);
		selectable_object_sptr_t s_this = p_this->register_to_engin();		
		if (s_this.get() == nullptr) {
			delete p_this;
		} 
		return dynamic_cast<chr_timer_t *>(s_this.get());
	}

    // no more use
    void abandon();

    // @once: true - the callback will only triggle once
    bool start(bool once = false);
    void stop();

private:
    chr_timer_t(uint32_t interval_ms, timer_callback_t &cb);

private:
    bool                m_running = false;
    uint32_t            m_interval_ms = 0;
    timer_callback_t    m_cb = nullptr;
    chan_selecter_t     m_selecter;
    chroutine_id_t      m_trigger_chroutine_id = INVALID_ID;    
    channel_t<int>::channel_sptr_t      m_trigger = nullptr;
    int                 m_d = 0;    // for channel read
    bool                m_once = false;
};

}

#endif