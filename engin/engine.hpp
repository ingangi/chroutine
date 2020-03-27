/// \file engin.hpp
/// 
/// engin is the manager for all chroutine_thread_t
///
/// \author ingangi
/// \version 0.1.0
/// \date 2018-12-12

#ifndef ENGINE_H
#define ENGINE_H

#include <map>
#include <vector>
#include "tools.hpp"
#include "chroutine.hpp"
#include "selectable_obj.hpp"

#ifdef ENABLE_HTTP_PLUGIN
#include "curl_stub.hpp"
#endif

#define ENGIN chr::engine_t::instance()
#define ENGINE_INIT(thrds) {ENGIN.init(thrds);}
#define YIELD() {ENGIN.yield();}
#define WAIT(t) {ENGIN.wait(t);}
#define SLEEP(t) {ENGIN.sleep(t);}
#define HOLD() {ENGIN.sleep(0xDC46C32800);}

namespace chr {

typedef std::map<std::thread::id, std::shared_ptr<chroutine_thread_t> > thread_pool_t;
typedef std::vector<std::shared_ptr<chroutine_thread_t> > thread_vector_t;
#ifdef ENABLE_HTTP_PLUGIN
typedef std::map<std::thread::id, selectable_object_sptr_t > http_stub_pool_t;
#endif


class chr_timer_t;
class engine_t final
{
    friend class chroutine_thread_t;
public:
    static engine_t& instance();
    ~engine_t();

    // start all thread, will block your thread until they are ready !!!
    void init(size_t init_pool_size);
    
    // yield myself by thread loop tick count
    void yield(int tick = 1);

    // yield myself for some time, waiting for son chroutine to be done.
    // will be awaken immediately when son is done, or time is out
    void wait(std::time_t wait_time_ms = 1000);
    
    // yield myself for some time, 
    // we should always use this instead of system sleep() !!!
    void sleep(std::time_t wait_time_ms);
    
    // create and run a chroutine in the lightest thread.
    chroutine_id_t create_chroutine(func_t func, void *arg);

    // create and run a son chroutine for the current chroutine.
    // returns the son's result so the father can get what he want.
    // @timeout_ms controls the max time for the son to run, 
    // if @timeout_ms is 0, that means father won't wait any time and doesn't care the result of son.
    reporter_base_t * create_son_chroutine(func_t func, const reporter_sptr_t & reporter, std::time_t timeout_ms);
    
    // called in a chroutine.
    // just start another chroutin in the same thread, 
    // father won't wait any time and doesn't care the result of son.
    chroutine_id_t create_son_chroutine(func_t func, void *arg);

    // register a select object to current thread
    int register_select_obj(const selectable_object_sptr_t & select_obj, std::thread::id thread_id);

    // unregister a select object from a thread
    int unregister_select_obj(selectable_object_it *key, std::thread::id thread_id);

    // get my chroutine id
    chroutine_id_t get_current_chroutine_id();

    // awake waiting chroutine
    int awake_chroutine(chroutine_id_t id);
    
    // awake waiting chroutine
    int awake_chroutine(std::thread::id thread_id, chroutine_id_t id);

    // the main thread
    void run();

    void stop_all();
    void stop_main();

#ifdef ENABLE_HTTP_PLUGIN
    // excute http req. thread safe after `m_init_over` become true
    std::shared_ptr<curl_rsp_t> exec_curl(const std::string & url
        , int connect_timeout = CURL_CON_TIME_OUT
        , int timeout = CURL_TIME_OUT
        , data_slot_func_t w_func = nullptr
        , void *w_func_handler = nullptr);
#endif

private:    
    engine_t();
    void on_thread_ready(size_t creating_index, std::thread::id thread_id);
    chroutine_thread_t *get_current_thread();
    chroutine_thread_t *get_lightest_thread();
    chroutine_thread_t *get_thread_by_id(std::thread::id thread_id);    
    
    // get current chroutine's reporter
    // (maybe no longer needed)
    reporter_base_t *get_my_reporter();
    
    // create and run a chroutine in the main thread.
    // main thread is for runtime tasks only
    chroutine_id_t create_chroutine_in_mainthread(func_t func, void *arg);

    // check threads availability
    // if thread was block, move it to a good one
    void check_threads();

private:
    std::mutex          m_pool_lock;            // only used during m_init_over is false
    thread_pool_t       m_pool;                 // is readonly after m_init_over become true
    thread_vector_t     m_creating;             // is readonly after m_init_over become true
    bool                m_init_over = false;    // if all threads ready
    int                 m_dispatch_seed = 0;
#ifdef ENABLE_HTTP_PLUGIN
    http_stub_pool_t    m_http_stubs;
#endif
    std::shared_ptr<chroutine_thread_t>     m_main_thread = nullptr;
    std::shared_ptr<chroutine_thread_t>     m_epoll_thread = nullptr;
    chr_timer_t*        m_flush_timer;
};

}
#endif