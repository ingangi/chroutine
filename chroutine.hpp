#ifndef CHROUTINE_H
#define CHROUTINE_H

#include <ucontext.h>
#include <mutex>
#include <memory>
#include <vector>
#include <string.h>
#include <iostream>

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
typedef struct chroutine_t{
    ucontext_t          ctx;
    func_t              func;
    void *              arg;
    chroutine_state_t   state;
    char *              stack;
    int                 yield_wait; // yield by frame count
    std::time_t         yield_to;   // yield until time
    chroutine_id_t      father;

    chroutine_t() {
        stack = new char[STACK_SIZE];
        //memset(stack, 0, STACK_SIZE);
        yield_wait = 0;
        yield_to = 0;
        father = INVALID_ID;
    }
    ~chroutine_t() {
        if (stack)
            delete [] stack;
    }
    int wait(time_t now) {
        if (yield_wait > 0)
            return yield_wait--;
        
        if (yield_to > 0 && yield_to > now)
            return 1;

        return 0;
    }
    void yield_over() {
        if (yield_to > 0) {
            std::cout << "wait time out!" << std::endl;
            yield_to = 0;
        }        
    }
    void son_finished() {
        std::cout << "son_finished!" << std::endl;
        yield_to = 0;
    }
} chroutine_t;

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

class chroutine_manager_t
{
public:
    static chroutine_manager_t& instance();
    static void yield(int wait = 1);
    static void wait(time_t wait_time_ms = 1000);
    ~chroutine_manager_t();

    chroutine_id_t create_chroutine(func_t func, void *arg);
    chroutine_id_t create_son_chroutine(func_t func, void *arg); // father is running chroutine

    std::time_t get_time_stamp();

    void start();
    void stop();
    bool is_running() {
        return m_is_running;
    }

private:
    chroutine_manager_t();
    int schedule();
    void yield_current(int wait);
    void wait_current(time_t wait_time_ms);
    
    static void entry(void *arg);

    void resume_to(chroutine_id_t id);
    bool done();
    chroutine_id_t pick_run_chroutine();
    chroutine_t * get_chroutine(chroutine_id_t id);

private:
    schedule_t m_schedule;
    bool m_is_running = false;
    bool m_need_stop = false;
};



#endif