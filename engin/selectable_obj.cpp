#include "selectable_obj.hpp"
#include "engine.hpp"

int selectable_object_it::register_to_engin()
{
    return ENGIN.register_select_obj(selectable_object_sptr_t(this));
}