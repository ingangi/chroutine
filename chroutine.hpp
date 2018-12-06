#ifndef CHROUTINE_H
#define CHROUTINE_H

#include <ucontext.h>
#include <mutex>
#include <memory>
#include <vector>
#include <string.h>
#include <iostream>
#include "reporter.hpp"

const unsigned int STACK_SIZE = 1024*128;
const int INVALID_ID = -1;
const int MAX_RUN_MS_EACH = 10;

typedef void (*func_t)(void *);

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
    chroutine_id_t yield_over();

    // called when son is done
    void son_finished();

    // get son's excute result and returned data
    reporter_base_t *get_reporter();

private:
    ucontext_t          ctx;
    func_t              func;
    void *              arg;
    chroutine_state_t   state;
    char *              stack;
    int                 yield_wait; // yield by frame count
    std::time_t         yield_to;   // yield until some time
    chroutine_id_t      father;
    chroutine_id_t      son;
    reporter_sptr_t     reporter;   // son chroutine excute result
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
    static std::shared_ptr<chroutine_thread_t> new_thread();
    void yield(int tick);
    void wait(std::time_t wait_time_ms);
    ~chroutine_thread_t();

    chroutine_id_t create_chroutine(func_t func, void *arg);
    chroutine_id_t create_son_chroutine(func_t func, reporter_sptr_t reporter); // son of the running chroutine

    void start(size_t creating_index);
    void stop();
    bool is_running() {
        return m_is_running;
    }
    
    reporter_base_t * get_current_reporter();

private:
    chroutine_thread_t();
    int schedule();
    void yield_current(int tick);
    void wait_current(std::time_t wait_time_ms);
    
    static void entry(void *arg);

    void resume_to(chroutine_id_t id);
    bool done();
    chroutine_id_t pick_run_chroutine();
    chroutine_t * get_chroutine(chroutine_id_t id);
    void remove_chroutine(chroutine_id_t id);

private:
    schedule_t m_schedule;
    bool m_is_running = false;
    bool m_need_stop = false;
    size_t m_creating_index = 0;
};



#endif