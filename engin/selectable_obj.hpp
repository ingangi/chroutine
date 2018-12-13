#ifndef SELECTALBLE_OBJ_H
#define SELECTALBLE_OBJ_H

#include <map>
#include <memory>

class selectable_object_it
{
public:
    virtual int select(int wait_ms) = 0;
};

typedef std::shared_ptr<selectable_object_it> selectable_object_sptr_t;
typedef std::map<void *, selectable_object_sptr_t> selectable_object_list_t;

#endif