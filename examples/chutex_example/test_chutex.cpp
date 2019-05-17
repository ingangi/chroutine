#include <map>
#include "chutex.hpp"
#include "engine.hpp"

using namespace chr;

int main(int argc, char **argv)
{
    ENGINE_INIT(1);

    std::map<int, int> m;
    chutex_t chutex;

    // 2 read chroutines
    ENGIN.create_chroutine([&](void *){
        SPDLOG(INFO, "reader 1 in thread:{}", std::this_thread::get_id());
        SLEEP(3010);
#if 0
        if (!chutex.try_lock()) {
            SPDLOG(INFO, "reader 1 trylock failed, exit");
            return;
        }
#else        
        chutex.lock();
#endif
        SPDLOG(INFO, "reader 1 get lock");
        for (auto iter = m.begin(); iter != m.end(); iter++) {
            SPDLOG(INFO, "reader 1; m[{}]:{}", iter->first, iter->second);
            SLEEP(1000);
        }
        m.clear();
        chutex.unlock();
        SPDLOG(INFO, "reader 1 unlock");
    }, nullptr);
    ENGIN.create_chroutine([&](void *){
        SPDLOG(INFO, "reader 2 in thread:{}", std::this_thread::get_id());
        SLEEP(3000);
        chutex_guard_t guard(chutex);
        SPDLOG(INFO, "reader 2 get lock");
        for (auto iter = m.begin(); iter != m.end(); iter++) {
            SPDLOG(INFO, "reader 2; m[{}]:{}", iter->first, iter->second);
            SLEEP(1000);
        }
        m.clear();
        SPDLOG(INFO, "reader 2 unlock");
    }, nullptr);
    
    
    // 1 write chroutine
    ENGIN.create_chroutine([&](void *){
        SPDLOG(INFO, "writer 1 in thread:{}", std::this_thread::get_id());
        chutex.lock();
        SPDLOG(INFO, "writer 1 get lock");
        for (int i = 0; i < 10; i++) {
            m[i] = i;
            SPDLOG(INFO, "writer 1; m[{}]:{}", i, i);
            SLEEP(100);
        }
        chutex.unlock();
        SPDLOG(INFO, "writer 1 unlock");
    }, nullptr);

    ENGIN.run();
}