#include "engine.hpp"

using namespace chr;

void test_resettle_sched() {
    ENGIN.create_chroutine([](void *){
        int i = 0;
        while (1) {
            SPDLOG(INFO, "I am 1, {} thread {}", i, readable_thread_id(std::this_thread::get_id()));
            i++;
            SLEEP(1000);
            // block happens!
            if (i == 5) {
                SPDLOG(INFO, "I am 1, block happens! thread {}", readable_thread_id(std::this_thread::get_id()));
                usleep(5000000);
                SPDLOG(INFO, "I am 1, block over! thread {}", readable_thread_id(std::this_thread::get_id()));
            }
        }
    }, nullptr);
    
    ENGIN.create_chroutine([](void *){
        int i = 0;
        while (1) {
            SPDLOG(INFO, "I am 2, {} thread {}", i, readable_thread_id(std::this_thread::get_id()));
            i++;
            SLEEP(1000);
        }
    }, nullptr);
    
    ENGIN.create_chroutine([](void *){
        int i = 0;
        while (1) {
            SPDLOG(INFO, "I am 3, {} thread {}", i, readable_thread_id(std::this_thread::get_id()));
            i++;
            SLEEP(1000);
        }
    }, nullptr);

}

void test_fair_sched() {
    ENGIN.create_chroutine([](void *){
        int i = 0;
        while (i<5) {
            SPDLOG(INFO, "I am 1");
            i++;
            YIELD();
        }
    }, nullptr);
    
    ENGIN.create_chroutine([](void *){
        int i = 0;
        while (i<5) {
            SPDLOG(INFO, "I am 2");
            i++;
            YIELD();
        }
    }, nullptr);
    
    ENGIN.create_chroutine([](void *){
        int i = 0;
        while (i<5) {
            SPDLOG(INFO, "I am 3");
            i++;
            YIELD();
        }
    }, nullptr);
    
    ENGIN.create_chroutine([](void *){
        int i = 0;
        while (i<5) {
            SPDLOG(INFO, "I am 4");
            i++;
            YIELD();
        }
    }, nullptr);
}

int main(int argc, char **argv)
{
    ENGINE_INIT(2);

    //test_fair_sched();
    test_resettle_sched();

    ENGIN.run();
}