#include <unistd.h>
#include "engine.hpp"


std::time_t get_time_stamp()
{
    std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
}

void thread_ms_sleep(uint32_t ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    while ((-1 == nanosleep(&ts, &ts)) && (EINTR == errno));
}


engine_t& engine_t::instance()
{
    static engine_t instance;
    return instance;
}

engine_t::engine_t()
{}

engine_t::~engine_t()
{
#ifdef ENABLE_HTTP_PLUGIN
    curl_global_cleanup();
#endif
}

void engine_t::init(size_t init_pool_size)
{
    if (!m_creating.empty())
        return;
    
    m_main_thread_id = std::this_thread::get_id();
    for (size_t i = 0; i < init_pool_size; i++) {
        std::shared_ptr<chroutine_thread_t> thrd = chroutine_thread_t::new_thread();
        m_creating.push_back(thrd);
        thrd.get()->start(i);
    }

    LOG("%s: init_pool_size = %d, m_main_thread_id = ", __FUNCTION__, init_pool_size)
        << m_main_thread_id << std::endl;

    while (!m_init_over) {
        thread_ms_sleep(10);
    }
    LOG << __FUNCTION__ << " OVER" << std::endl;
}

void engine_t::on_thread_ready(size_t creating_index, std::thread::id thread_id)
{
    std::lock_guard<std::mutex> lck (m_pool_lock);
    if (creating_index > m_creating.size() - 1) {
        return;
    }

    m_pool[thread_id] = m_creating[creating_index];
    // LOG << __FUNCTION__ << ". run in thread:" << std::this_thread::get_id() << " thread ready:" << thread_id << std::endl;
    if (m_pool.size() == m_creating.size()) {
        m_init_over = true; 
        LOG << __FUNCTION__ << " m_init_over now is TRUE" << std::endl;

#ifdef ENABLE_HTTP_PLUGIN
        curl_global_init(CURL_GLOBAL_ALL);
        for (auto it = m_pool.begin(); it != m_pool.end(); it++) {
            selectable_object_sptr_t s_this = curl_stub_t::create(it->first);
            if (s_this.get()) {
                m_http_stubs[it->first] = s_this;
            }
        }
#endif
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
    // check called in main thread
    if (m_main_thread_id != std::this_thread::get_id()) {
        LOG << __FUNCTION__ << " error: not called in main thread!\n";
        return INVALID_ID;
    }
    chroutine_thread_t *pthrd = get_lightest_thread();
    if (pthrd == nullptr)
        return INVALID_ID;

    return pthrd->create_chroutine(func, arg);
}

reporter_base_t * engine_t::create_son_chroutine(func_t func, const reporter_sptr_t & reporter, std::time_t timeout_ms)
{
    if (timeout_ms == 0) {
        create_son_chroutine(func, nullptr);
        return nullptr;
    }

    chroutine_thread_t *pthrd = get_current_thread();
    if (pthrd == nullptr)
        return nullptr;

    pthrd->create_son_chroutine(func, reporter);
    pthrd->wait(timeout_ms);
    return pthrd->get_current_reporter();
}

chroutine_id_t engine_t::create_son_chroutine(func_t func, void *arg)
{
    chroutine_thread_t *pthrd = get_current_thread();
    if (pthrd == nullptr)
        return INVALID_ID;

    return pthrd->create_chroutine(func, arg);
}

chroutine_thread_t *engine_t::get_current_thread()
{
    if (!m_init_over) {
        LOG << __FUNCTION__ << " failed: m_init_over FALSE" << std::endl;
        return nullptr;
    }
    //std::lock_guard<std::mutex> lck (m_pool_lock);
    const auto& iter = m_pool.find(std::this_thread::get_id());
    if (iter == m_pool.end())
        return nullptr;

    return iter->second.get();
}

chroutine_thread_t *engine_t::get_thread_by_id(std::thread::id thread_id)
{
    if (!m_init_over) {
        LOG << __FUNCTION__ << " failed: m_init_over FALSE" << std::endl;
        return nullptr;
    }
    
    const auto& iter = m_pool.find(thread_id);
    if (iter == m_pool.end())
        return nullptr;

    return iter->second.get();
}

chroutine_thread_t *engine_t::get_lightest_thread()
{
    //std::lock_guard<std::mutex> lck (m_pool_lock);
    if (!m_init_over) {
        LOG << __FUNCTION__ << " failed: m_init_over FALSE" << std::endl;
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

int engine_t::register_select_obj(const selectable_object_sptr_t & select_obj, std::thread::id thread_id)
{
    chroutine_thread_t *pthrd = nullptr;
    if (NULL_THREAD_ID == thread_id)
        pthrd = get_current_thread();
    else
        pthrd = get_thread_by_id(thread_id);

    if (pthrd == nullptr)
        return -1;

    // LOG << __FUNCTION__ << " run in thread:" << std::this_thread::get_id() << std::endl;
    pthrd->register_selector(select_obj);
    return 0;
}

int engine_t::unregister_select_obj(selectable_object_it *key, std::thread::id thread_id)
{
    chroutine_thread_t *pthrd = nullptr;
    if (NULL_THREAD_ID == thread_id)
        pthrd = get_current_thread();
    else
        pthrd = get_thread_by_id(thread_id);

    if (pthrd == nullptr)
        return -1;

    pthrd->unregister_selector(key);
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


#ifdef ENABLE_HTTP_PLUGIN
std::shared_ptr<curl_rsp_t> engine_t::exec_curl(const std::string & url
    , int connect_timeout
    , int timeout
    , data_slot_func_t w_func
    , void *w_func_handler)
{
    if (!m_init_over) {
        LOG << __FUNCTION__ << " failed: m_init_over FALSE" << std::endl;
        return nullptr;
    }
    
    const auto& iter = m_http_stubs.find(std::this_thread::get_id());
    if (iter == m_http_stubs.end()){
        LOG << __FUNCTION__ << " failed: cant find http_stub" << std::endl;
        return nullptr;
    }

    curl_stub_t *stub = dynamic_cast<curl_stub_t *>(iter->second.get());
    if (stub == nullptr) {
        LOG << __FUNCTION__ << " failed: curl_stub_t * is nullptr" << std::endl;
        return nullptr;
    }

    //LOG << "thread:" << std::this_thread::get_id() << " exec_curl:" << url << std::endl;
    return stub->exec_curl(url, connect_timeout, timeout, w_func, w_func_handler);
}
#endif

void engine_t::run()
{
    while(1) {
        // todo: preemption for chroutines that exceed 10ms
        thread_ms_sleep(500);
    }
}