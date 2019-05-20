
#include <sstream>
#include "tools.hpp"

namespace chr {
    
std::time_t get_time_stamp()
{
    std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
}

void thread_ms_sleep(uint32_t ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    while ((-1 == nanosleep(&ts, &ts)) && (EINTR == errno));
}

std::string readable_thread_id(const std::thread::id & id)
{
    std::stringstream sin;
    sin << id;
    return sin.str();
}

}