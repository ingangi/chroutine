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

namespace chr {

// it's a spin lock
class chutex_t final
{
public:
    chutex_t(){}
    ~chutex_t(){}

    void lock();

    // can unlock by others
    void unlock();

    // 
    bool try_lock();
    
private:
    chutex_t(const chutex_t&) = delete;
    chutex_t(chutex_t&&) = delete;
    chutex_t& operator=(const chutex_t&) = delete;
    chutex_t& operator=(chutex_t&&) = delete;

private:
    std::atomic<bool> m_flag = ATOMIC_VAR_INIT(false);    //true:locked
};

class chutex_guard_t final
{
public:
    chutex_guard_t(chutex_t &chtex) : m_chtex(chtex){
        m_chtex.lock();
    }
    ~chutex_guard_t(){
        m_chtex.unlock();
    }
private:
    chutex_t &m_chtex;
};

}

#endif