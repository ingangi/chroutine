
#include "chroutine.hpp"
#include <unistd.h>
#include <thread>
#include <iostream>

chroutine_manager_t& chroutine_manager_t::instance()
{
    static chroutine_manager_t instance;
    return instance;
}

chroutine_manager_t::chroutine_manager_t()
{
}

chroutine_manager_t::~chroutine_manager_t()
{}

void chroutine_manager_t::yield(int wait)
{
    chroutine_manager_t::instance().yield_current(wait);
}

void chroutine_manager_t::wait(time_t wait_time_ms)
{
    chroutine_manager_t::instance().wait_current(wait_time_ms);
}

chroutine_t * chroutine_manager_t::get_chroutine(chroutine_id_t id)
{
    if (id < 0 || id > int(m_schedule.chroutines.size()) - 1)
        return nullptr;

    return m_schedule.chroutines[id].get();
}

void chroutine_manager_t::entry(void *arg)
{
    chroutine_manager_t *p_this = static_cast<chroutine_manager_t *>(arg);
    if (p_this == nullptr)
        return;

    chroutine_t * p_c = p_this->get_chroutine(p_this->m_schedule.running_id);
    if (p_c == nullptr)
        return;

    //std::cout << "entry start, " << p_this->m_schedule.running_id << " left:" << p_this->m_schedule.chroutines.size()  << std::endl;
    p_c->state = chroutine_state_running;
    p_c->func(p_c->arg);
    //p_c->state = chroutine_state_fin;
    p_this->m_schedule.chroutines_to_free.push_back(p_this->m_schedule.chroutines[p_this->m_schedule.running_id]);
    p_this->m_schedule.chroutines.erase(p_this->m_schedule.chroutines.begin() + p_this->m_schedule.running_id);
    p_this->m_schedule.running_id = INVALID_ID;

    if (p_c->father != INVALID_ID) {
        chroutine_t * father = p_this->get_chroutine(p_c->father);
        if (father != nullptr) {
            father->son_finished();
        }
    }
    //std::cout << "entry over, " << p_this->m_schedule.running_id << " left:" << p_this->m_schedule.chroutines.size() << std::endl;
}

chroutine_id_t chroutine_manager_t::create_chroutine(func_t func, void *arg)
{
    if (func == nullptr) 
        return INVALID_ID;

    std::shared_ptr<chroutine_t> c(new chroutine_t);
    chroutine_t *p_c = c.get();
    if (p_c == nullptr)
        return INVALID_ID;

    getcontext(&(p_c->ctx));
    p_c->ctx.uc_stack.ss_sp = p_c->stack;
    p_c->ctx.uc_stack.ss_size = STACK_SIZE;
    p_c->ctx.uc_stack.ss_flags = 0;
    p_c->ctx.uc_link = &(m_schedule.main);
    p_c->func = func;
    p_c->arg = arg;
    p_c->state = chroutine_state_ready;
    makecontext(&(p_c->ctx),(void (*)(void))(entry), 1, this);

    chroutine_id_t id = INVALID_ID;
    {
        //std::lock_guard<std::mutex> lck (m_schedule.mutex);
        m_schedule.chroutines.push_back(c);
        id = m_schedule.chroutines.size() - 1;
    }

    //std::cout << "create_chroutine over, " << id << std::endl;
    return id;
}

chroutine_id_t chroutine_manager_t::create_son_chroutine(func_t func, void *arg)
{
    //std::cout << "create_son_chroutine start, " << m_schedule.running_id << std::endl;

    if (m_schedule.running_id == INVALID_ID)
        return INVALID_ID;

    chroutine_id_t son = create_chroutine(func, arg);
    if (son == INVALID_ID)
        return INVALID_ID;
    
    chroutine_t * pson = m_schedule.chroutines[son].get();
    if (pson == nullptr)
        return INVALID_ID;

    pson->father = m_schedule.running_id;
    //std::cout << "create_son_chroutine over, " << son << std::endl;
    return son;
}

void chroutine_manager_t::yield_current(int wait)
{
    if (wait <= 0)
        return;

    if (m_schedule.running_id < 0 || m_schedule.running_id > int(m_schedule.chroutines.size())-1)
        return;
    
    chroutine_t * co = m_schedule.chroutines[m_schedule.running_id].get();
    if (co == nullptr || co->state != chroutine_state_running)
        return;
    
    //std::cout << "yield_current ..." << m_schedule.running_id << std::endl;
    co->state = chroutine_state_suspend;
    co->yield_wait += wait;
    m_schedule.running_id = INVALID_ID;
    swapcontext(&(co->ctx), &(m_schedule.main));
}

void chroutine_manager_t::wait_current(time_t wait_time_ms)
{
    if (wait_time_ms <= 0)
        return;

    if (m_schedule.running_id < 0 || m_schedule.running_id > int(m_schedule.chroutines.size())-1)
        return;
    
    chroutine_t * co = m_schedule.chroutines[m_schedule.running_id].get();
    if (co == nullptr || co->state != chroutine_state_running)
        return;
    
    //std::cout << "wait_current ..." << m_schedule.running_id << std::endl;
    co->state = chroutine_state_suspend;
    co->yield_to = get_time_stamp() + wait_time_ms;
    m_schedule.running_id = INVALID_ID;
    swapcontext(&(co->ctx), &(m_schedule.main));
}

bool chroutine_manager_t::done()
{
    return m_schedule.chroutines.empty();
}

void chroutine_manager_t::resume_to(chroutine_id_t id)
{
    if (id < 0 || id > int(m_schedule.chroutines.size())-1)
        return;

    chroutine_t * co = m_schedule.chroutines[id].get();
    if (co == nullptr || co->state != chroutine_state_suspend)
        return;
    
    //std::cout << "resume_to ..." << id << std::endl;
    swapcontext(&(m_schedule.main),&(co->ctx));
    //std::cout << "resume_to ..." << id << " over" << std::endl;
}

chroutine_id_t chroutine_manager_t::pick_run_chroutine()
{
    // clean finished nodes
    m_schedule.chroutines_to_free.clear();

    if (m_schedule.running_id != INVALID_ID)
        return m_schedule.running_id;

    if (m_schedule.chroutines.empty())
        return INVALID_ID;

    chroutine_id_t index = INVALID_ID;
    chroutine_id_t i = INVALID_ID;
    chroutine_t *p_c = nullptr;
    time_t now = get_time_stamp();
    for (auto &node : m_schedule.chroutines) {
        i++;
        if (node.get()->wait(now) > 0)
            continue;
        if (p_c == nullptr) {
            p_c = node.get();
            index = i;
        }
    }

    if (p_c) {
        //std::cout << "pick_run_chroutine ..." << index << std::endl;
        p_c->yield_over();
        p_c->state = chroutine_state_running;
        m_schedule.running_id = index;
        swapcontext(&(m_schedule.main),&(p_c->ctx));
        //std::cout << "pick_run_chroutine ..." << index << " over" << std::endl;
    }
    return index;
}

int chroutine_manager_t::schedule()
{
    m_is_running = true;
    std::cout << "chroutine_manager_t::schedule is_running " << m_is_running << std::endl;
    while (!m_need_stop) {
        pick_run_chroutine();
        if (done())
            usleep(10000);
    }
    m_is_running = false;
    std::cout << "chroutine_manager_t::schedule is_running " << m_is_running << std::endl;
    return 0;
}

void chroutine_manager_t::start()
{
    if (m_is_running)
        return;

    std::thread thrd( [this] { this->schedule(); } );
    thrd.detach();
}

void chroutine_manager_t::stop()
{
    m_need_stop = true;
}

std::time_t chroutine_manager_t::get_time_stamp()
{
    std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
}