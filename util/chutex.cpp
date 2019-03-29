#include "chutex.hpp"
#include "engine.hpp"


void chutex_t::lock()
{
    bool expected = false;
    while(!m_flag.compare_exchange_weak(expected, true, std::memory_order_acquire)) {
        expected = false;
        YIELD();
    }
}

void chutex_t::unlock() 
{
    m_flag.store(false, std::memory_order_release);
}

bool chutex_t::try_lock() 
{
    bool expected = false;
    return m_flag.compare_exchange_strong(expected, true, std::memory_order_acquire);
}