/// \file selectable_obj.hpp
/// 
/// selectable_object_it is the base class for all IO objects.
/// we should use selectable_object_it instead of system IO APIs !
///
/// \author ingangi
/// \version 0.1.0
/// \date 2018-12-24

#ifndef SELECTALBLE_OBJ_H
#define SELECTALBLE_OBJ_H

#include <map>
#include <memory>
#include <thread>

const std::thread::id NULL_THREAD_ID;

class selectable_object_it;
typedef std::shared_ptr<selectable_object_it> selectable_object_sptr_t;
typedef std::map<void *, selectable_object_sptr_t> selectable_object_list_t;

class selectable_object_it
{
public:
    virtual ~selectable_object_it(){}
    virtual int select(int wait_ms) = 0;

    // after register to engin, select will be scheduled.    
    selectable_object_sptr_t register_to_engin(std::thread::id thread_id = NULL_THREAD_ID); // Once called, the life cycle is left to engin !!!  
    int unregister_from_engin(std::thread::id thread_id = NULL_THREAD_ID);
};


#endif