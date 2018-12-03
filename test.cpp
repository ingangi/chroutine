#include "chroutine.hpp"
#include <iostream>
#include <unistd.h>
#include <thread>



void fun_1()
{
    int tick = 0;
    while (1) {
        tick++;
        std::cout << "fun_1 tick = " << tick << " (" << std::this_thread::get_id() << ")" << std::endl;
        usleep(100000);

        if (tick % 3 == 0)
            chroutine_manager_t::yield();
    }
}

void fun_2()
{
    int tick = 0;
    while (1) {
        tick++;
        std::cout << "fun_2 tick = " << tick << " (" << std::this_thread::get_id() << ")" << std::endl;
        usleep(100000);

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