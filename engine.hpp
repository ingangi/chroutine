#ifndef ENGINE_H
#define ENGINE_H

#include "chroutine.hpp"

typedef std::map<std::thread::id, std::shared_ptr<chroutine_thread_t> > thread_pool_t;
typedef std::vector<std::shared_ptr<chroutine_thread_t> > creating_threads_t;

class engine_t 
{
public:
    static engine_t& instance();
    ~engine_t();
    void init(size_t init_pool_size);

    void on_thread_ready(size_t creating_index, std::thread::id thread_id);

private:    
    engine_t();

private:
    std::mutex          m_pool_lock;
    thread_pool_t       m_pool;
    creating_threads_t  m_creating;
};

#endif