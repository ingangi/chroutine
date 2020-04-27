/// \file redis_client.hpp
/// 
///
/// \author ingangi
/// \version 0.1.0
/// \date 2019-04-27

#pragma once

#include "selectable_obj.hpp"

namespace chr {

// A curl_stub_t is reused by all requests in a chroutine_thread_t 
class redis_client : public selectable_object_it
{
public:
    redis_client();
    virtual ~redis_client();
    virtual int select(int wait_ms);
};

}
#endif
