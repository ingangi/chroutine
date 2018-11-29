
#include "chroutine.hpp"

chroutine_manager_t& chroutine_manager_t::getInstance()
{
    static chroutine_manager_t instance;
    return instance;
}

chroutine_manager_t::chroutine_manager_t()
{}

chroutine_manager_t::~chroutine_manager_t()
{}

chroutine_id_t chroutine_manager_t::create_chroutine(func_t func, void *arg)
{
    if (func == nullptr) 
        return -1;

    std::shared_ptr<chroutine_t> c(new chroutine_t);
    chroutine_t *p_c = c.get();
    if (pc == nullptr)
        return -1;

    getcontext(&(p_c->ctx));
    p_c->ctx.uc_stack.ss_sp = p_c->stack;
    p_c->ctx.uc_stack.ss_size = STACK_SIZE;
    p_c->ctx.uc_stack.ss_flags = 0;
    p_c->ctx.uc_link = &(m_schedule.main);
    makecontext(p_c,(void (*)(void))func,1,arg);

    chroutine_id_t id = INVALID_ID;
    {
        std::lock_guard<std::mutex> lck (m_schedule.mutex);
        m_schedule.chroutines.push_back(c);
        id = m_schedule.chroutines.size() - 1;
    }

    

    return id;
}

void chroutine_manager_t::yield_current()
{
    if (m_schedule.running_id < 0 || m_schedule.running_id > int(chroutines.size())-1)
        return;
    
    chroutine_t * co = m_schedule.chroutines[id].get();
    if (co == nullptr || co->state != chroutine_state_running)
        return;
    
    co->state = chroutine_state_suspend;
    m_schedule.running_id = INVALID_ID;
    swapcontext(&(co->ctx), &(m_schedule.main));
}

void chroutine_manager_t::resume_to(chroutine_id_t id)
{
    if (id > int(chroutines.size())-1)
        return;

    chroutine_t * co = m_schedule.chroutines[id].get();
    if (co == nullptr || co->state != chroutine_state_suspend)
        return;
    
    swapcontext(&(m_schedule.main),&(co->ctx));
}

bool chroutine_manager_t::done()
{
    return chroutines.empty();
}