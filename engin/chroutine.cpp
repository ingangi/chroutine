#include <unistd.h>
#include <iostream>
#include "chroutine.hpp"
#include "engine.hpp"


chroutine_t::chroutine_t() 
{
    stack = new char[STACK_SIZE];
}

chroutine_t::~chroutine_t() 
{
    if (stack)
        delete [] stack;
}

int chroutine_t::wait(std::time_t now) 
{
    if (yield_wait > 0)
        return yield_wait--;
    
    if (yield_to != 0 && yield_to > now)
        return 1;

    return 0;
}

chroutine_id_t chroutine_t::yield_over(son_result_t result) 
{
    chroutine_id_t timeout_chroutine = INVALID_ID;
    if (yield_to != 0 && stop_son_when_yield_over) {
        //LOG << "wait time out!" << std::endl;
        if (reporter.get()) {
            reporter.get()->set_result(result);
        }
        timeout_chroutine = son;
        son = INVALID_ID;
        stop_son_when_yield_over = false;
    }

    yield_to = 0;
    return timeout_chroutine;
}

void chroutine_t::son_finished() 
{
    LOG << "son_finished!" << std::endl;
    if (reporter.get()) {
        reporter.get()->set_result(result_done);
    }
    yield_to = 0;
}

reporter_base_t *chroutine_t::get_reporter() 
{
    return reporter.get();
}

std::shared_ptr<chroutine_thread_t> chroutine_thread_t::new_thread()
{
    return std::shared_ptr<chroutine_thread_t>(new chroutine_thread_t());
}

chroutine_thread_t::chroutine_thread_t()
{
}

chroutine_thread_t::~chroutine_thread_t()
{}

void chroutine_thread_t::yield(int tick)
{
    yield_current(tick);
}

void chroutine_thread_t::wait(std::time_t wait_time_ms)
{
    wait_current(wait_time_ms, true);
}

void chroutine_thread_t::sleep(std::time_t wait_time_ms)
{
    wait_current(wait_time_ms, false);
}

chroutine_t * chroutine_thread_t::get_chroutine(chroutine_id_t id)
{
    if (id < 0 || id > int(m_schedule.chroutines.size()) - 1)
        return nullptr;

    return m_schedule.chroutines[id].get();
}

void chroutine_thread_t::remove_chroutine(chroutine_id_t id)
{
    if (id < 0 || id > int(m_schedule.chroutines.size()) - 1)
        return;
        
    m_schedule.chroutines_to_free.push_back(m_schedule.chroutines[id]);
    m_schedule.chroutines.erase(m_schedule.chroutines.begin() + id);
}

reporter_base_t * chroutine_thread_t::get_current_reporter()
{
    chroutine_t *p_c = get_chroutine(m_schedule.running_id);
    if (p_c == nullptr)
        return nullptr;

    return p_c->get_reporter();
}

void chroutine_thread_t::entry(void *arg)
{
    chroutine_thread_t *p_this = static_cast<chroutine_thread_t *>(arg);
    if (p_this == nullptr)
        return;

    chroutine_t * p_c = p_this->get_chroutine(p_this->m_schedule.running_id);
    if (p_c == nullptr)
        return;

    //LOG << "entry start, " << p_this->m_schedule.running_id << " left:" << p_this->m_schedule.chroutines.size()  << std::endl;
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
    //LOG << "entry over, " << p_this->m_schedule.running_id << " left:" << p_this->m_schedule.chroutines.size() << std::endl;
}

chroutine_id_t chroutine_thread_t::create_chroutine(func_t & func, void *arg)
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
    p_c->func = std::move(func);
    p_c->arg = arg;
    p_c->state = chroutine_state_ready;
    makecontext(&(p_c->ctx),(void (*)(void))(entry), 1, this);

    chroutine_id_t id = INVALID_ID;
    {
        //std::lock_guard<std::mutex> lck (m_schedule.mutex);
        m_schedule.chroutines.push_back(c);
        id = m_schedule.chroutines.size() - 1;
    }

    //LOG << "create_chroutine over, " << id << std::endl;
    return id;
}

chroutine_id_t chroutine_thread_t::create_son_chroutine(func_t & func, const reporter_sptr_t & reporter)
{
    //LOG << "create_son_chroutine start, " << m_schedule.running_id << std::endl;

    chroutine_t * pfather = get_chroutine(m_schedule.running_id);
    if (pfather == nullptr)
        return INVALID_ID;
    
    pfather->reporter = reporter;

    chroutine_id_t son = create_chroutine(func, reporter.get()->get_data());
    if (son == INVALID_ID)
        return INVALID_ID;
    
    chroutine_t * pson = m_schedule.chroutines[son].get();
    if (pson == nullptr)
        return INVALID_ID;

    pson->father = m_schedule.running_id;
    pfather->son = son;
    //LOG << "create_son_chroutine over, " << son << std::endl;
    return son;
}

void chroutine_thread_t::yield_current(int tick)
{
    if (tick <= 0)
        return;

    if (m_schedule.running_id < 0 || m_schedule.running_id > int(m_schedule.chroutines.size())-1)
        return;
    
    chroutine_t * co = m_schedule.chroutines[m_schedule.running_id].get();
    if (co == nullptr || co->state != chroutine_state_running)
        return;
    
    //LOG << "yield_current ..." << m_schedule.running_id << std::endl;
    co->state = chroutine_state_suspend;
    co->yield_wait += tick;
    m_schedule.running_id = INVALID_ID;
    swapcontext(&(co->ctx), &(m_schedule.main));
}

void chroutine_thread_t::wait_current(std::time_t wait_time_ms, bool stop_son_after_wait)
{
    if (wait_time_ms <= 0)
        return;

    if (m_schedule.running_id < 0 || m_schedule.running_id > int(m_schedule.chroutines.size())-1)
        return;
    
    chroutine_t * co = m_schedule.chroutines[m_schedule.running_id].get();
    if (co == nullptr || co->state != chroutine_state_running)
        return;
    
    //LOG << "wait_current ..." << m_schedule.running_id << std::endl;
    co->state = chroutine_state_suspend;
    co->yield_to = get_time_stamp() + wait_time_ms;
    co->stop_son_when_yield_over = stop_son_after_wait;
    m_schedule.running_id = INVALID_ID;
    swapcontext(&(co->ctx), &(m_schedule.main));
}

bool chroutine_thread_t::done()
{
    return m_schedule.chroutines.empty();
}

void chroutine_thread_t::resume_to(chroutine_id_t id)
{
    if (id < 0 || id > int(m_schedule.chroutines.size())-1)
        return;

    chroutine_t * co = m_schedule.chroutines[id].get();
    if (co == nullptr || co->state != chroutine_state_suspend)
        return;
    
    //LOG << "resume_to ..." << id << std::endl;
    swapcontext(&(m_schedule.main),&(co->ctx));
    //LOG << "resume_to ..." << id << " over" << std::endl;
}

chroutine_id_t chroutine_thread_t::pick_run_chroutine()
{
    // clean finished nodes
    m_schedule.chroutines_to_free.clear();

    if (m_schedule.running_id != INVALID_ID)
        return m_schedule.running_id;

    if (m_schedule.chroutines.empty())
        return INVALID_ID;

    chroutine_id_t index = INVALID_ID;
    chroutine_t *p_c = nullptr;
    std::time_t now = get_time_stamp();
    size_t size = m_schedule.chroutines.size();
    chroutine_id_t i = m_schedule.last_run_id + 1;
    if (i < 0) i = 0;

    for (; (size_t)i < size; i++) {
        auto &node = m_schedule.chroutines[i];
        if (node.get()->wait(now) > 0)
            continue;
        if (p_c == nullptr) {
            p_c = node.get();
            index = i;
        }
    }

    if (p_c) {
        //LOG << "pick_run_chroutine ..." << index << std::endl;
        remove_chroutine(p_c->yield_over());  // remove time out son chroutin
        p_c->state = chroutine_state_running;
        m_schedule.running_id = index;
        swapcontext(&(m_schedule.main),&(p_c->ctx));
        //LOG << "pick_run_chroutine ..." << index << " over" << std::endl;
    }
    m_schedule.last_run_id = index;
    return index;
}

int chroutine_thread_t::schedule()
{
    m_is_running = true;
    LOG << "chroutine_thread_t::schedule is_running " << m_is_running << std::endl;
    engine_t::instance().on_thread_ready(m_creating_index, std::this_thread::get_id());
    while (!m_need_stop) {        
        int processed = 0;
        processed += select_all();
        processed += pick_run_chroutine() == INVALID_ID ? 0 : 1;
        if (processed == 0)
            thread_ms_sleep(10);
    }
    m_is_running = false;
    LOG << "chroutine_thread_t::schedule is_running " << m_is_running << std::endl;
    return 0;
}

void chroutine_thread_t::start(size_t creating_index)
{
    if (m_is_running)
        return;

    m_creating_index = creating_index;
    std::thread thrd( [this] { this->schedule(); } );
    thrd.detach();
}

void chroutine_thread_t::stop()
{
    m_need_stop = true;
}

int chroutine_thread_t::select_all()
{
    int processed = 0;
    for (auto iter = m_selector_list.begin(); iter != m_selector_list.end(); iter++) {
        selectable_object_it *p_obj = iter->second.get();
        if (p_obj) {
            // LOG << "selecting.." << p_obj << ", thread:" << std::this_thread::get_id() << std::endl;
            processed += p_obj->select(0);
        }
    }
    return processed;
}

void chroutine_thread_t::register_selector(const selectable_object_sptr_t & select_obj)
{
    void *key = select_obj.get();
    if (key) {
        auto iter = m_selector_list.find(key);
        if (iter == m_selector_list.end()) {
            m_selector_list[key] = select_obj;
            // LOG << __FUNCTION__ << " thread:" << std::this_thread::get_id() << " OK: key = " << key << std::endl;
        } else {
            // LOG << __FUNCTION__ << " thread:" << std::this_thread::get_id() << " failed: key already exist: " << key << std::endl;
        }
    }
}

void chroutine_thread_t::unregister_selector(const selectable_object_sptr_t & select_obj)
{
    unregister_selector(select_obj.get());
}

void chroutine_thread_t::unregister_selector(selectable_object_it *p_obj)
{
    void *key = p_obj;
    auto iter = m_selector_list.find(key);
    if (iter == m_selector_list.end()) {
        LOG << __FUNCTION__ << " failed: key not exist: " << key << std::endl;
    } else {
        m_selector_list.erase(iter);
        LOG << __FUNCTION__ << " OK: key = " << key << std::endl;
    }
}


int chroutine_thread_t::awake_chroutine(chroutine_id_t id)
{
    chroutine_t * p_c = get_chroutine(id);
    if (p_c == nullptr) {
        LOG << __FUNCTION__ << " p_c == nullptr ! id = " << id << std::endl;
        return -1;
    }

    remove_chroutine(p_c->yield_over(result_done));
    return 0;
}