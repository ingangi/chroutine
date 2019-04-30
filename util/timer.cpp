#include "timer.hpp"

namespace chr {

chr_timer_t::chr_timer_t()
{
    m_trigger = channel_t<int>::create();
}

chr_timer_t::~chr_timer_t()
{}


int chr_timer_t::select(int wait_ms)
{
    if (!m_running) {
        return 0;
    }

    // @wait_ms will be ignored, as 0
    return m_selecter.select_once();
}

bool chr_timer_t::start(uint32_t interval_ms, timer_callback_t& cb)
{
    if (m_running) {
        LOG << "chr_timer_t::start ignored: already running\n";
        return false;
    }

    m_cb = std::move(cb);

    if (m_cb == nullptr || interval_ms == 0) {
        LOG << "chr_timer_t::start error: m_cb == nullptr || interval_ms == 0\n";
        return false;
    }

    // start the trigger chroutine

    // add select case    
    int d = 0;
    m_selecter.add_case(m_trigger.get(), &d, [&](){
        LOG << "timer triggled !!!" << std::endl;
        ENGIN.create_son_chroutine([&](void *){
            if (m_cb != nullptr) m_cb();
        }, nullptr);
    });

    m_running = true;
    return true;
}

void chr_timer_t::stop()
{
    if (!m_running) {
        LOG << "chr_timer_t::stop ignored: not running\n";
        return;
    }
    m_running = false;
    m_cb = nullptr;

    // stop the trigger chroutine

}

}