#include "selectable_obj.hpp"
#include "engine.hpp"

selectable_object_sptr_t selectable_object_it::register_to_engin(std::thread::id thread_id)
{
    selectable_object_sptr_t s_this(this);
    if (ENGIN.register_select_obj(s_this, thread_id) == 0)
        return s_this;

    return nullptr;
}

int selectable_object_it::unregister_from_engin(std::thread::id thread_id)
{
    return ENGIN.unregister_select_obj(this, thread_id);
}