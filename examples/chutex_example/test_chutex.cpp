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
        LOG << "reader 1 in thread:" << std::this_thread::get_id() << std::endl;
        SLEEP(3010);
#if 0
        if (!chutex.try_lock()) {
            LOG << "reader 1 trylock failed, exit\n";
            return;
        }
#else        
        chutex.lock();
#endif
        LOG << "reader 1 get lock\n";
        for (auto iter = m.begin(); iter != m.end(); iter++) {
            LOG("reader 1; m[%d]:%d", iter->first, iter->second);
            SLEEP(1000);
        }
        m.clear();
        chutex.unlock();
        LOG << "reader 1 unlock\n";
    }, nullptr);
    ENGIN.create_chroutine([&](void *){
        LOG << "reader 2 in thread:" << std::this_thread::get_id() << std::endl;
        SLEEP(3000);
        chutex_guard_t guard(chutex);
        LOG << "reader 2 get lock\n";
        for (auto iter = m.begin(); iter != m.end(); iter++) {
            LOG("reader 2; m[%d]:%d", iter->first, iter->second);
            SLEEP(1000);
        }
        m.clear();
        LOG << "reader 2 unlock\n";
    }, nullptr);
    
    
    // 1 write chroutine
    ENGIN.create_chroutine([&](void *){
        LOG << "writer 1 in thread:" << std::this_thread::get_id() << std::endl;
        chutex.lock();
        LOG << "writer 1 get lock\n";
        for (int i = 0; i < 10; i++) {
            m[i] = i;
            LOG("writer 1; m[%d]:%d", i, i);
            SLEEP(100);
        }
        chutex.unlock();
        LOG << "writer 1 unlock\n";
    }, nullptr);

    ENGIN.run();    
    LOG << "over ..." << std::endl;
}