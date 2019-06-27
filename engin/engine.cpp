#include <unistd.h>
#include <signal.h>
#include "engine.hpp"
#include "timer.hpp"

void signal_handle(int signal_num){
    ENGIN.stop_all();
}

namespace chr {
    
const static int MAX_ENTRY_CALL_TIME_MS = 500;
const static int ENTRY_CALL_CHECK_TIMER_MS = MAX_ENTRY_CALL_TIME_MS/2;


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

    SPDLOG(INFO, "{}: init_pool_size = {}, m_main_thread_id = {}", __FUNCTION__, init_pool_size, readable_thread_id(m_main_thread_id));

    while (!m_init_over) {
        thread_ms_sleep(10);
    }
    
    // main thread do not need start()
    m_main_thread = chroutine_thread_t::new_thread();
    SPDLOG(INFO, "{}: OVER", __FUNCTION__);
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
        SPDLOG(INFO, "{}: m_init_over now is TRUE", __FUNCTION__);

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
    // if (m_main_thread_id != std::this_thread::get_id()) {
    //     SPDLOG(ERROR, "{} error: not called in main thread!", __FUNCTION__);
    //     return INVALID_ID;
    // }
    chroutine_thread_t *pthrd = get_lightest_thread();
    if (pthrd == nullptr)
        return INVALID_ID;

    return pthrd->create_chroutine(func, arg);
}

chroutine_id_t engine_t::create_chroutine_in_mainthread(func_t func, void *arg)
{
    if (m_main_thread)
        return m_main_thread->create_chroutine(func, arg);        

    return INVALID_ID;
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
        SPDLOG(ERROR, "{} failed: m_init_over FALSE", __FUNCTION__);
        return nullptr;
    }

    std::thread::id cur_id = std::this_thread::get_id();
    if (cur_id == m_main_thread_id) {
        return m_main_thread.get();
    }

    const auto& iter = m_pool.find(cur_id);
    if (iter == m_pool.end())
        return nullptr;

    return iter->second.get();
}

chroutine_thread_t *engine_t::get_thread_by_id(std::thread::id thread_id)
{
    if (!m_init_over) {
        SPDLOG(ERROR, "{} failed: m_init_over FALSE", __FUNCTION__);
        return nullptr;
    }
    
    const auto& iter = m_pool.find(thread_id);
    if (iter == m_pool.end())
        return nullptr;

    return iter->second.get();
}

chroutine_thread_t *engine_t::get_lightest_thread()
{
    if (!m_init_over) {
        SPDLOG(ERROR, "{} failed: m_init_over FALSE", __FUNCTION__);
        return nullptr;
    }

    if (m_creating.empty())
        return nullptr;

    // let's return a random one for test
    return m_creating[m_dispatch_seed++ % m_creating.size()].get();
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


// awake waiting chroutine
int engine_t::awake_chroutine(std::thread::id thread_id, chroutine_id_t id)
{
    chroutine_thread_t *pthrd = get_thread_by_id(thread_id);
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
        SPDLOG(ERROR, "{} failed: m_init_over FALSE", __FUNCTION__);
        return nullptr;
    }
    
    const auto& iter = m_http_stubs.find(std::this_thread::get_id());
    if (iter == m_http_stubs.end()){
        SPDLOG(ERROR, "{} failed: cant find http_stub", __FUNCTION__);
        return nullptr;
    }

    curl_stub_t *stub = dynamic_cast<curl_stub_t *>(iter->second.get());
    if (stub == nullptr) {
        SPDLOG(ERROR, "{} failed: curl_stub_t * is nullptr", __FUNCTION__);
        return nullptr;
    }

    return stub->exec_curl(url, connect_timeout, timeout, w_func, w_func_handler);
}
#endif

void engine_t::check_threads()
{
    std::time_t now = get_time_stamp();
    thread_vector_t goods;
    thread_vector_t bads;
    for (auto it = m_pool.begin(); it != m_pool.end(); it++) {
        auto &thrd = it->second;
        if (thrd) {
            std::time_t thrd_entry_time = thrd->entry_time();
            if (thrd_entry_time != 0) {
                SPDLOG(TRACE, "engine_t::check_thread: {:p}, entry time: {}, , now time: {}"
                    , (void*)(thrd.get())
                    , thrd_entry_time
                    , now);
            }

            if (thrd_entry_time != 0 && now > MAX_ENTRY_CALL_TIME_MS + thrd_entry_time) {
                if (thread_state_t_running == thrd->state()) {
                    bads.push_back(thrd);
                }
            } else {
                if (thread_state_t_blocking == thrd->state()) {
                    thrd->set_state(thread_state_t_running); //TODO: more smart
                } else {
                    goods.push_back(thrd);
                }
            }
        }
    }

    if (bads.size() > 0) {
        size_t goodsize = goods.size();
        if (goodsize == 0) {
            SPDLOG(CRITICAL, "threads need switch, but no good threads left!!!");
            return;
        }

        int goodindex = 0;
        for (auto &bad : bads) {
            bad->move_chroutines_to_thread(goods[goodindex++ % goodsize]);
        }
    }
}

void engine_t::stop_main()
{
    create_chroutine_in_mainthread([this](void *){
        // clean timer
        if (m_flush_timer) {
            m_flush_timer->stop();
            m_flush_timer->abandon();
            m_flush_timer = nullptr;
        }
        // clean others

        // stop the main sched
        if (m_main_thread) {
            m_main_thread->stop();
        }
    }, nullptr);
}

void engine_t::stop_all()
{
    // stop pool
    for (auto it = m_pool.begin(); it != m_pool.end(); it++) {
        auto &thrd = it->second;
        if (thrd) {
            thrd->stop();
        }
    }
    // todo: join

    // end main
    stop_main();
}

void engine_t::run()
{
    if (!m_main_thread) {
        SPDLOG(ERROR, "main thread<chroutine_thread_t> is null!");
        return;
    }

    if (m_main_thread->is_running()) {
        SPDLOG(ERROR, "main thread run failed: already running!");
        return;
    }

    signal(SIGINT, signal_handle);
    signal(SIGQUIT, signal_handle);
    signal(SIGTERM, signal_handle);
    m_main_thread->set_main_thread_flag(true);
    SPDLOG(DEBUG, "main thread is about to run, check the id:{}=={}", readable_thread_id(m_main_thread_id), readable_thread_id(std::this_thread::get_id()));

    create_chroutine_in_mainthread([this](void *){
        while (true) {
            // max error value ENTRY_CALL_CHECK_TIMER_MS
            m_main_thread->sleep(ENTRY_CALL_CHECK_TIMER_MS);
            check_threads();
        }        
    }, nullptr);

    // async logger flush
#ifdef DEBUG_BUILD
    uint32_t flush_timer_ms = 500;
#else
    uint32_t flush_timer_ms = 5000;
#endif
    m_flush_timer = chr_timer_t::create(flush_timer_ms, [](){
        LOG.flush();
    });
    if (m_flush_timer) {
        m_flush_timer->start();
    }

    m_main_thread->schedule();

    // clean
    SPDLOG(INFO, "main thread exited!");
}

}