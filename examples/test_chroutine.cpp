#include <iostream>
#include <unistd.h>
#include <thread>
#include "engine.hpp"

using namespace chr;

void fake_io_work(int costtime = 10)
{    
    std::time_t now = get_time_stamp();
    while (get_time_stamp() - now < costtime) {
        SLEEP(20);
    }
}

typedef struct test_return_data_t{
    std::string b;
    int a;
}test_return_data_t;

void fun_1(void *arg)
{
    int tick = 0;
    while (1) {
        tick++;
        SPDLOG(INFO, "fun_1 tick = {}: ({})", tick, readable_thread_id(std::this_thread::get_id()));
        fake_io_work(1000);

        if (tick % 3 == 0) {
            // example of sync with son chroutine: 
            // call a `son` chroutine in the `father` chroutine, and get the result after son is done.

            // if timeout happens during son's running, son will be stopped and removed immediately            
            reporter_base_t * rpt = ENGIN.create_son_chroutine([](void *d) {
                test_return_data_t *data = (test_return_data_t *)d;
                SPDLOG(INFO, "son-of-func_1 ({})", readable_thread_id(std::this_thread::get_id()));
                fake_io_work(5000);
                data->a = 888;
                data->b = "hello father";
                SPDLOG(INFO, "son-of-func_1 ({}) OVER", readable_thread_id(std::this_thread::get_id()));
            }, reporter_t<test_return_data_t>::create(), 5100);

            // son is over, check the result
            if (rpt) {
                SPDLOG(INFO, "fun_1 ({}) finish wait, son result:{}", readable_thread_id(std::this_thread::get_id()), rpt->get_result());
                if (rpt->get_result() == result_done) {
                    test_return_data_t *p_data = static_cast<test_return_data_t*>(rpt->get_data());
                    SPDLOG(INFO, "son chroutine is done, test_return_data_t.a={}, b={}", p_data->a, p_data->b);
                }
            }
        }
    }
}

void fun_3(void *arg)
{
    while (1) {
        //SLEEP(2000);
        usleep(710000);  //testing block os thread
        break;
    }
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

    // ENGIN.create_chroutine(fun_1, nullptr);  
    // ENGIN.create_chroutine([](void *){
    // int tick = 0;
    //     while (1) {
    //         SPDLOG(INFO, "fun_2 tick = {} ({})", ++tick, readable_thread_id(std::this_thread::get_id()));
    //         SLEEP(1000);

    //         // create another chroutine in the same thread
    //         if (tick == 2) {
    //             SPDLOG(INFO, "try to create another chroutin");
    //             ENGIN.create_son_chroutine(fun_3, nullptr);
    //         }
    //     }
    // }, nullptr);

    test_fair_sched();

    ENGIN.run();
}