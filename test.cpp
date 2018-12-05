#include "chroutine.hpp"
#include <iostream>
#include <unistd.h>
#include <thread>

void fake_io_work(int costtime = 10)
{    
    std::time_t now = chroutine_manager_t::instance().get_time_stamp();
    while (chroutine_manager_t::instance().get_time_stamp() - now < costtime) {
        usleep(1000);
        chroutine_manager_t::yield();
    }
}

typedef struct {
    int a;
    std::string b;
}return_data;
void fun_3(return_data *data)
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
            chroutine_manager_t::instance().create_son_chroutine(func_t(fun_3), reporter_t<return_data>::create());

            std::cout << "fun_1 (" << std::this_thread::get_id() << ") start wait" << std::endl;
            chroutine_manager_t::wait(5010);
            reporter_base_t * rpt = chroutine_manager_t::instance().get_current_reporter();
            std::cout << "fun_1 (" << std::this_thread::get_id() << ") finish wait, son result:" << rpt->get_result() << std::endl;

            if (rpt->get_result() == result_done) {
                return_data *p_data = static_cast<return_data*>(rpt->get_data());
                std::cout << "son chroutine is done, return_data.a=" << p_data->a << ", b=" << p_data->b << std::endl;
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
    chroutine_manager_t &mng = chroutine_manager_t::instance();
    mng.create_chroutine(func_t(fun_1), nullptr);  
    mng.create_chroutine(func_t(fun_2), nullptr);
            
    mng.start();

    while(1) {
        usleep(500000);
    }
    
    std::cout << "over ..." << std::endl;
}