#include "engine.hpp"
#include "timer.hpp"

using namespace chr;

int main(int argc, char **argv)
{
    ENGINE_INIT(1);

    ENGIN.create_chroutine([&](void *){
        chr_timer_t* timer = chr_timer_t::create(1000, [](){
            SPDLOG(INFO, "i am called by timer");
        });

        timer->start();

        SLEEP(5050);

        timer->stop();

        SPDLOG(INFO, "run once more!");
        timer->start(true);
        SLEEP(1550);

        timer->abandon();   // will make the engin to delete the timer

        SPDLOG(INFO, "test chroutine exit");
    }, nullptr);

    ENGIN.run();
}