/// \file chroutine.hpp
/// 
/// defined core classes or structs of the `engin`
/// `chroutine_t` indicades a croutine, 
/// `chroutine_thread_t` holds a os thread which the croutines run on
///
/// \author ingangi
/// \version 0.1.0
/// \date 2018-12-12

#ifndef CHROUTINE_H
#define CHROUTINE_H

#include <ucontext.h>
#include <mutex>
#include <memory>
#include <vector>
#include <string.h>
#include <iostream>
#include <functional>
#include "reporter.hpp"
#include "selectable_obj.hpp"
#include "logger.hpp"

const unsigned int STACK_SIZE = 1024*128;
const int INVALID_ID = -1;
const int MAX_RUN_MS_EACH = 10;

typedef std::function<void(void *)> func_t;

typedef enum {
    //chroutine_state_free = 0,
    chroutine_state_ready = 0,
    chroutine_state_running,
    chroutine_state_suspend,
    //chroutine_state_fin,

} chroutine_state_t;

typedef int chroutine_id_t;
class chroutine_t
{
    friend class chroutine_thread_t;

public:
    chroutine_t();
    ~chroutine_t();

    // if wait is over, return 0
    int wait(std::time_t now);

    // called when resume, return the timeout son id if exist
    chroutine_id_t yield_over(son_result_t result = result_timeout);

    // called when son is done
    void son_finished();

    // get son's excute result and returned data
    reporter_base_t *get_reporter();

private:
    ucontext_t          ctx;
    func_t              func = nullptr;
    void *              arg = nullptr;
    chroutine_state_t   state = chroutine_state_suspend;
    char *              stack = nullptr;
    int                 yield_wait = 0; // yield by frame count
    std::time_t         yield_to = 0;   // yield until some time
    chroutine_id_t      father = INVALID_ID;
    chroutine_id_t      son = INVALID_ID;
    reporter_sptr_t     reporter;   // son chroutine excute result
    bool                stop_son_when_yield_over = false;
};


typedef std::vector<std::shared_ptr<chroutine_t> > chroutine_list_t;

typedef struct schedule_t {
    //std::mutex          mutex;
    ucontext_t          main;
    chroutine_id_t      running_id;
    chroutine_list_t    chroutines;
    chroutine_list_t    chroutines_to_free;

    schedule_t() 
    : running_id(INVALID_ID)
    {}    
}schedule_t;

// chroutine_thread_t hold a os thread 
// and a list of chroutines run in the thread.
class chroutine_thread_t
{
public:
    ~chroutine_thread_t();

    // create self
    static std::shared_ptr<chroutine_thread_t> new_thread();

    // yield current chroutin for @tick frames
    void yield(int tick);
    
    // yield current chroutin for at most @wait_time_ms time
    // when timeout happens, stop the son chroutine and continue running.
    void wait(std::time_t wait_time_ms);
    
    // yield current chroutin for at most @wait_time_ms time
    // when timeout happens, continue running.
    void sleep(std::time_t wait_time_ms);

    // create a chroutine
    chroutine_id_t create_chroutine(func_t & func, void *arg);
    
    // create a son chroutine of current chroutine
    chroutine_id_t create_son_chroutine(func_t & func, const reporter_sptr_t & reporter);

    // start the thread
    void start(size_t creating_index);
    
    // start the thread
    void stop();

    bool is_running() {
        return m_is_running;
    }
    
    // get the excuse result of the son chroutine, usually call after wait.
    reporter_base_t * get_current_reporter();

    // register/unregister selectable objects
    void register_selector(const selectable_object_sptr_t & select_obj);
    void unregister_selector(const selectable_object_sptr_t & select_obj);
    void unregister_selector(selectable_object_it *p_obj);

    
    chroutine_id_t get_running_id() {
        return m_schedule.running_id;
    }
    
    // awake waiting chroutine
    int awake_chroutine(chroutine_id_t id);

private:
    chroutine_thread_t();

    // the while loop of the thread
    int schedule();

    // called by yield
    void yield_current(int tick);

    // called by wait/sleep
    void wait_current(std::time_t wait_time_ms, bool stop_son_after_wait);
    
    // where the funcs of chroutines were called
    static void entry(void *arg);

    // swap chroutines, not used, see pick_run_chroutine
    void resume_to(chroutine_id_t id);

    // whether all chroutines were finished (list empty)
    bool done();

    // chroutine schdule, pick next chroutine to run (swap happens)
    chroutine_id_t pick_run_chroutine();

    // get chroutine pointer by id
    chroutine_t * get_chroutine(chroutine_id_t id);
    
    // remove chroutine by id
    void remove_chroutine(chroutine_id_t id);

    // select all selectable_object_it
    // rpc/tcp/http/pipe for this thread
    int select_all();

private:
    schedule_t                  m_schedule;
    bool                        m_is_running = false;
    bool                        m_need_stop = false;
    size_t                      m_creating_index = 0;
    selectable_object_list_t    m_selector_list;
};

#endif