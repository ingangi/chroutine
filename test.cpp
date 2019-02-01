#include <iostream>
#include <unistd.h>
#include <thread>
#include "engine.hpp"


void fake_io_work(int costtime = 10)
{    
    std::time_t now = get_time_stamp();
    while (get_time_stamp() - now < costtime) {
        usleep(1000);
        YIELD();
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
        std::cout << "fun_1 tick = " << tick << " (" << std::this_thread::get_id() << ")" << std::endl;
        fake_io_work(1000);

        if (tick % 3 == 0) {
            // example of sync with son chroutine: 
            // call a `son` chroutine in the `father` chroutine, and get the result after son is done.

            // if timeout happens during son's running, son will be stopped and removed immediately            
            reporter_base_t * rpt = ENGIN.create_son_chroutine([](void *d) {
                test_return_data_t *data = (test_return_data_t *)d;
                std::cout << "son-of-func_1 (" << std::this_thread::get_id() << ")" << std::endl;
                fake_io_work(5000);
                data->a = 888;
                data->b = "hello father";
                std::cout << "son-of-func_1 (" << std::this_thread::get_id() << ") OVER" << std::endl;
            }, reporter_t<test_return_data_t>::create(), 5010);

            // son is over, check the result
            if (rpt) {
                std::cout << "fun_1 (" << std::this_thread::get_id() << ") finish wait, son result:" << rpt->get_result() << std::endl;
                if (rpt->get_result() == result_done) {
                    test_return_data_t *p_data = static_cast<test_return_data_t*>(rpt->get_data());
                    std::cout << "son chroutine is done, test_return_data_t.a=" << p_data->a << ", b=" << p_data->b << std::endl;
                }
            }
        }
    }
}


int main(int argc, char **argv)
{   
    ENGINE_INIT(2);

    ENGIN.create_chroutine(fun_1, nullptr);  
    ENGIN.create_chroutine([](void *){
    int tick = 0;
        while (1) {
            std::cout << "fun_2 tick = " << ++tick << " (" << std::this_thread::get_id() << ")" << std::endl;
            fake_io_work(2000);
        }
    }, nullptr);

    while(1) {
        usleep(500000);
    }
    
    std::cout << "over ..." << std::endl;
}