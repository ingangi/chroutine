#include "engine.hpp"
#include "channel.hpp"

using namespace chr;

int main(int argc, char **argv)
{
    ENGINE_INIT(2);

    auto chan_sptr = channel_t<int>::create(10);
    auto &chan = *(chan_sptr.get());

    // 2 read chroutines
    ENGIN.create_chroutine([&](void *){
        LOG << "reader 1 in thread:" << std::this_thread::get_id() << std::endl;
        for (int i=0; i < 20; i++) {
            int r = -1;
            LOG << "[" << i << "]reader 1 try to read r ..." << std::endl;
            chan >> r;
            LOG << "[" << i << "]reader 1 read r:" << r << std::endl;
        }
    }, nullptr);
    ENGIN.create_chroutine([&](void *){
        LOG << "reader 2 in thread:" << std::this_thread::get_id() << std::endl;
        for (int i=0; i < 20; i++) {
            int r = -1;
            LOG << "[" << i << "]reader 2 try to read r ..." << std::endl;
            chan >> r;
            LOG << "[" << i << "]reader 2 read r:" << r << std::endl;
        }
    }, nullptr);
    
    
    // 1 write chroutine
    ENGIN.create_chroutine([&](void *){
        LOG << "writer 1 in thread:" << std::this_thread::get_id() << std::endl;
        SLEEP(3000);
        for (int i=0; i < 30; i++) {
            LOG << "[" << i << "]try to write chan" << std::endl;
            chan << i;
            LOG << "[" << i << "]write chan over" << std::endl;
            SLEEP(1000);
        }
    }, nullptr);

    ENGIN.run();    
    LOG << "over ..." << std::endl;
}