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

typedef struct return_data_t{
    std::string b;
    int a;
}return_data_t;

void fun_3(return_data_t *data)
{
    std::cout << "fun_3 (" << std::this_thread::get_id() << ")" << std::endl;
    fake_io_work(5000);
    data->a = 888;
    data->b = "hello father";
    std::cout << "fun_3 (" << std::this_thread::get_id() << ") OVER" << std::endl;
}

void fun_1()
{
    int tick = 0;
    while (1) {
        tick++;
        std::cout << "fun_1 tick = " << tick << " (" << std::this_thread::get_id() << ")" << std::endl;
        fake_io_work(1000);

        if (tick % 3 == 0) {
            // example of sync with son chroutine:
            ENGIN.create_son_chroutine(func_t(fun_3), reporter_t<return_data_t>::create());
            std::cout << "fun_1 (" << std::this_thread::get_id() << ") start wait" << std::endl;
            WAIT(5010);
            reporter_base_t * rpt = ENGIN.get_my_reporter();
            std::cout << "fun_1 (" << std::this_thread::get_id() << ") finish wait, son result:" << rpt->get_result() << std::endl;

            if (rpt->get_result() == result_done) {
                return_data_t *p_data = static_cast<return_data_t*>(rpt->get_data());
                std::cout << "son chroutine is done, return_data_t.a=" << p_data->a << ", b=" << p_data->b << std::endl;
            }

        }
    }
}

void fun_2()
{
    int tick = 0;
    while (1) {
        std::cout << "fun_2 tick = " << ++tick << " (" << std::this_thread::get_id() << ")" << std::endl;
        fake_io_work(2000);
    }
}


int main(int argc, char **argv)
{   
    ENGINE_INIT(2);
    
    ENGIN.create_chroutine(func_t(fun_1), nullptr);  
    ENGIN.create_chroutine(func_t(fun_2), nullptr);

    while(1) {
        usleep(500000);
    }
    
    std::cout << "over ..." << std::endl;
}