#include "engine.hpp"
#include "timer.hpp"

using namespace chr;

int main(int argc, char **argv)
{
    ENGINE_INIT(1);

    ENGIN.create_chroutine([&](void *){
        chr_timer_t* timer = chr_timer_t::create(1000, [](){
            LOG << "i am called by timer:" << get_time_stamp() << std::endl;
        });

        timer->start();

        SLEEP(5000);
        LOG << "test chroutine CONTINUE\n";

        timer->stop();

        timer->destroy();   // will make the engin to delete the timer

        LOG << "test chroutine exit\n";
    }, nullptr);

    ENGIN.run();    
    LOG << "over ..." << std::endl;
}