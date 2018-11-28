
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
    return 0;
}

void chroutine_manager_t::yield_current()
{}

void chroutine_manager_t::resume_to(chroutine_id_t id)
{}

bool chroutine_manager_t::done()
{
    return true;
}