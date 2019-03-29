/// \file chutex.hpp
/// 
/// mutex for chroutine
///
/// \author ingangi
/// \version 0.1.0
/// \date 2019-03-28

#ifndef CHUTEX_HPP
#define CHUTEX_HPP

#include <atomic>
#include "engine.hpp"

// it's a spin lock
class chutex_t final
{
public:
    chutex_t(){}
    ~chutex_t(){}

    void lock() {
        bool expected = false;
        while(!m_flag.compare_exchange_weak(expected, true, std::memory_order_acquire)) {
            expected = false;
            YIELD();
        }
    }

    // FIXME: can unlock by others
    void unlock() {
        m_flag.store(false, std::memory_order_release);
    }

    // 
    bool try_lock() {
        bool expected = false;
        return m_flag.compare_exchange_strong(expected, true, std::memory_order_acquire);
    }
    
private:
    chutex_t(const chutex_t&) = delete;
    chutex_t(chutex_t&&) = delete;
    chutex_t& operator=(const chutex_t&) = delete;
    chutex_t& operator=(chutex_t&&) = delete;

private:
    std::atomic<bool> m_flag = ATOMIC_VAR_INIT(false);    //true:locked
};


#endif