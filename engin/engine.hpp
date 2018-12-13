#ifndef ENGINE_H
#define ENGINE_H

#include <map>
#include <thread>
#include "chroutine.hpp"

typedef std::map<std::thread::id, std::shared_ptr<chroutine_thread_t> > thread_pool_t;
typedef std::vector<std::shared_ptr<chroutine_thread_t> > creating_threads_t;

#define ENGIN engine_t::instance()
#define ENGINE_INIT(thrds) {ENGIN.init(thrds);}
#define YIELD() {ENGIN.yield();}
#define WAIT(t) {ENGIN.wait(t);}

std::time_t get_time_stamp();

class engine_t 
{
    friend class chroutine_thread_t;
public:
    static engine_t& instance();
    ~engine_t();

    // start all thread, will block your thread until they ready !!!
    void init(size_t init_pool_size);
    
    // yield myself by thread loop tick count
    void yield(int tick = 1);

    // yield myself for some time, waiting for son chroutine to be done.
    // will be awaken immediately when son is done, or time is out
    void wait(std::time_t wait_time_ms = 1000);
    
    // create a chroutine in the lightest thread
    chroutine_id_t create_chroutine(func_t func, void *arg);

    // create a son chroutine for the current chroutin
    chroutine_id_t create_son_chroutine(func_t func, reporter_sptr_t reporter);

    // get current chroutine's reporter
    reporter_base_t *get_my_reporter();

private:    
    engine_t();
    void on_thread_ready(size_t creating_index, std::thread::id thread_id);
    chroutine_thread_t *get_current_thread();
    chroutine_thread_t *get_lightest_thread();

private:
    std::mutex          m_pool_lock;            // only used during m_init_over is false
    thread_pool_t       m_pool;                 // is readonly after m_init_over become true
    creating_threads_t  m_creating;             // is readonly after m_init_over become true
    bool                m_init_over = false;    // if all thread ready
};

#endif