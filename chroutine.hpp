#ifndef CHROUTINE_H
#define CHROUTINE_H

#include <ucontext.h>

const unsigned int STACK_SIZE = 1024*10;

typedef void (*func_t)(void *);

typedef enum {
    chroutine_state_free = 0,
    chroutine_state_ready = 1,
    chroutine_state_running = 2,
    chroutine_state_suspend = 3,

} chroutine_state_t;

typedef struct {
    ucontext_t          ctx;
    func_t              func;
    void *              arg;
    chroutine_state_t   state;
    char                stack[STACK_SIZE];
} chroutine_t;

typedef std::vector<chroutine_t *> chroutine_list_t;
typedef int chroutine_id_t;

typedef struct {
    ucontext_t          main;
    chroutine_id_t      running_id;
    chroutine_list_t    chroutines;

    schedule_t() : running_id(-1) {        
    }    
}schedule_t;

class chroutine_manager_t
{
public:
    chroutine_manager_t& chroutine_manager_t::instance();
    ~chroutine_manager_t();

    chroutine_id_t create_chroutine(func_t func, void *arg);
    void yield_current();
    void resume_to(chroutine_id_t id);
    bool done();

private:
    chroutine_manager_t();

private:
    schedule_t m_schedule;
};



#endif