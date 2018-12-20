#include "test_server.hpp"
#include "engine.hpp"
#include <unistd.h>


void fun_1()
{
    test_rpc_server *server = new test_rpc_server;
    if (server->start("0.0.0.0:50061") != 0) {
        std::cout << "test_rpc_server start failed\n";
        delete server;
    }
}

int main(int argc, char **argv)
{   
    ENGINE_INIT(1);
    ENGIN.create_chroutine(func_t(fun_1), nullptr);
    while(1) {
        usleep(500000);
    }    
    std::cout << "over ..." << std::endl;
}