/// \file chutex.hpp
/// 
/// tool funcs
///
/// \author ingangi
/// \version 0.1.0
/// \date 2019-05-20

#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <thread>

namespace chr {

std::time_t get_time_stamp();
void thread_ms_sleep(uint32_t ms);
std::string readable_thread_id(const std::thread::id & id);

}

#endif