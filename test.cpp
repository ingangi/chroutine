#include "chroutine.hpp"
#include <iostream>
#include <unistd.h>
#include <thread>



void fun_1()
{
    std::cout << "fun_1 1" << "(" << std::this_thread::get_id() << ")" << std::endl;
    usleep(100000);

    std::cout << "fun_1 2" << "(" << std::this_thread::get_id() << ")" << std::endl;
    usleep(100000);

    chroutine_manager_t::yield(2);

    std::cout << "fun_1 3" << "(" << std::this_thread::get_id() << ")" << std::endl;
    usleep(100000);

    std::cout << "fun_1 4" << "(" << std::this_thread::get_id() << ")" << std::endl;
    usleep(100000);

    std::cout << "fun_1 5" << "(" << std::this_thread::get_id() << ")" << std::endl;
}

void fun_2()
{
    std::cout << "fun_2 1" << "(" << std::this_thread::get_id() << ")" << std::endl;
    usleep(100000);

    std::cout << "fun_2 2" << "(" << std::this_thread::get_id() << ")" << std::endl;
    usleep(100000);

    std::cout << "fun_2 3" << "(" << std::this_thread::get_id() << ")" << std::endl;
    usleep(100000);

    chroutine_manager_t::yield();

    std::cout << "fun_2 4" << "(" << std::this_thread::get_id() << ")" << std::endl;
    usleep(100000);
    
    std::cout << "fun_2 5" << "(" << std::this_thread::get_id() << ")" << std::endl;
}

int main(int argc, char **argv)
{   
    chroutine_manager_t &mng = chroutine_manager_t::instance();
    mng.create_chroutine(func_t(fun_1), nullptr);  
    mng.create_chroutine(func_t(fun_2), nullptr);
            
    while(1) {
        usleep(500000);
    }
    
    std::cout << "over ..." << std::endl;
}