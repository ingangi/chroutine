#include <unistd.h>
#include "engine.hpp"


std::time_t get_time_stamp()
{
    std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
}

engine_t& engine_t::instance()
{
    static engine_t instance;
    return instance;
}

engine_t::engine_t()
{}

engine_t::~engine_t()
{}

void engine_t::init(size_t init_pool_size)
{
    if (!m_creating.empty())
        return;
        
    for (size_t i = 0; i < init_pool_size; i++) {
        std::shared_ptr<chroutine_thread_t> thrd = chroutine_thread_t::new_thread();
        m_creating.push_back(thrd);
        thrd.get()->start(i);
    }

    while (!m_init_over) {
        usleep(10000);
    }
    std::cout << __FUNCTION__ << " OVER" << std::endl;
}

void engine_t::on_thread_ready(size_t creating_index, std::thread::id thread_id)
{
    std::lock_guard<std::mutex> lck (m_pool_lock);
    if (creating_index > m_creating.size() - 1) {
        return;
    }

    m_pool[thread_id] = m_creating[creating_index];
    if (m_pool.size() == m_creating.size()) {
        m_init_over = true; 
        std::cout << __FUNCTION__ << " m_init_over now is TRUE" << std::endl;
    }
}

void engine_t::yield(int tick)
{
    chroutine_thread_t *pthrd = get_current_thread();
    if (pthrd == nullptr)
        return;

    pthrd->yield(tick);
}

void engine_t::wait(std::time_t wait_time_ms)
{    
    chroutine_thread_t *pthrd = get_current_thread();
    if (pthrd == nullptr)
        return;

    pthrd->wait(wait_time_ms);
}

void engine_t::sleep(std::time_t wait_time_ms)
{    
    chroutine_thread_t *pthrd = get_current_thread();
    if (pthrd == nullptr)
        return;

    pthrd->sleep(wait_time_ms);
}

chroutine_id_t engine_t::create_chroutine(func_t func, void *arg)
{    
    chroutine_thread_t *pthrd = get_lightest_thread();
    if (pthrd == nullptr)
        return INVALID_ID;

    return pthrd->create_chroutine(func, arg);
}

reporter_base_t * engine_t::create_son_chroutine(func_t func, reporter_sptr_t reporter, std::time_t timeout_ms)
{
    chroutine_thread_t *pthrd = get_current_thread();
    if (pthrd == nullptr)
        return nullptr;

    pthrd->create_son_chroutine(func, reporter);
    pthrd->wait(timeout_ms);
    return pthrd->get_current_reporter();
}

chroutine_thread_t *engine_t::get_current_thread()
{
    if (!m_init_over) {
        std::cout << __FUNCTION__ << " failed: m_init_over FALSE" << std::endl;
        return nullptr;
    }
    //std::lock_guard<std::mutex> lck (m_pool_lock);
    const auto& iter = m_pool.find(std::this_thread::get_id());
    if (iter == m_pool.end())
        return nullptr;

    return iter->second.get();
}

chroutine_thread_t *engine_t::get_lightest_thread()
{
    //std::lock_guard<std::mutex> lck (m_pool_lock);
    if (!m_init_over) {
        std::cout << __FUNCTION__ << " failed: m_init_over FALSE" << std::endl;
        return nullptr;
    }

    if (m_creating.empty())
        return nullptr;

    // let's return a random one for test
    return m_creating[time(NULL) % m_creating.size()].get();
}

reporter_base_t *engine_t::get_my_reporter()
{
    chroutine_thread_t *pthrd = get_current_thread();
    if (pthrd == nullptr)
        return nullptr;

    return pthrd->get_current_reporter();
}

int engine_t::register_select_obj(selectable_object_sptr_t select_obj)
{
    chroutine_thread_t *pthrd = get_current_thread();
    if (pthrd == nullptr)
        return -1;

    pthrd->register_selector(select_obj);
    return 0;
}

chroutine_id_t engine_t::get_current_chroutine_id()
{
    chroutine_thread_t *pthrd = get_current_thread();
    if (pthrd == nullptr)
        return INVALID_ID;

    return pthrd->get_running_id();
}

int engine_t::awake_chroutine(chroutine_id_t id)
{
    chroutine_thread_t *pthrd = get_current_thread();
    if (pthrd == nullptr)
        return -1;

    return pthrd->awake_chroutine(id);
}