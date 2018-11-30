#ifndef CHROUTINE_H
#define CHROUTINE_H

#include <ucontext.h>
#include <mutex>
#include <memory>
#include <vector>

const unsigned int STACK_SIZE = 1024*50;
const int INVALID_ID = -1;
const int MAX_RUN_MS_EACH = 10;

typedef void (*func_t)(void *);

typedef enum {
    //chroutine_state_free = 0,
    chroutine_state_ready = 0,
    chroutine_state_running,
    chroutine_state_suspend,

} chroutine_state_t;

typedef struct {
    ucontext_t          ctx;
    func_t              func;
    void *              arg;
    chroutine_state_t   state;
    char                stack[STACK_SIZE];
} chroutine_t;

typedef std::vector<std::shared_ptr<chroutine_t> > chroutine_list_t;
typedef int chroutine_id_t;

typedef struct schedule_t {
    //std::mutex          mutex;
    ucontext_t          main;
    chroutine_id_t      running_id;
    chroutine_list_t    chroutines;
    //std::time_t         run_start_time;

    schedule_t() 
    : running_id(INVALID_ID)
    // , run_start_time(0) 
    {}    
}schedule_t;

class chroutine_manager_t
{
public:
    static chroutine_manager_t& instance();
    static void yield();
    ~chroutine_manager_t();

    chroutine_id_t create_chroutine(func_t func, void *arg);

    std::time_t get_time_stamp();

private:
    chroutine_manager_t();
    int schedule();
    void yield_current();
    void start();
    
    static void entry(void *arg);

    void resume_to(chroutine_id_t id);
    bool done();
    chroutine_id_t pick_run_chroutine();
    chroutine_t * get_chroutine(chroutine_id_t id);

private:
    schedule_t m_schedule;
};



#endif