#include "chroutine.hpp"
#include <iostream>
#include <unistd.h>
#include <thread>

void fun_3()
{
    std::cout << "fun_3 (" << std::this_thread::get_id() << ")" << std::endl;
    std::time_t now = chroutine_manager_t::instance().get_time_stamp();
    while (chroutine_manager_t::instance().get_time_stamp() - now < 9000) {
        usleep(10000);
        chroutine_manager_t::yield();
    }

    std::cout << "fun_3 (" << std::this_thread::get_id() << ") OVER" << std::endl;
}

void fun_1()
{
    int tick = 0;
    while (1) {
        tick++;
        std::cout << "fun_1 tick = " << tick << " (" << std::this_thread::get_id() << ")" << std::endl;
        usleep(1000000);

        if (tick % 3 == 0) {
            chroutine_manager_t::instance().create_son_chroutine(func_t(fun_3), nullptr);
            std::cout << "fun_1 (" << std::this_thread::get_id() << ") start wait" << std::endl;
            chroutine_manager_t::wait(10000);
            std::cout << "fun_1 (" << std::this_thread::get_id() << ") finish wait" << std::endl;
        }
    }
}

void fun_2()
{
    int tick = 0;
    while (1) {
        tick++;
        std::cout << "fun_2 tick = " << tick << " (" << std::this_thread::get_id() << ")" << std::endl;
        usleep(10000);

        if (tick % 5 == 0)
            chroutine_manager_t::yield();
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