#include "test_server.hpp"
#include "engine.hpp"
#include <unistd.h>


int main(int argc, char **argv)
{   
    ENGINE_INIT(1);
    ENGIN.create_chroutine([](void *){
        test_rpc_server *server = new test_rpc_server;
        if (server->start("0.0.0.0:50061") != 0) {
            std::cout << "test_rpc_server start failed\n";
            delete server;
        }
    }, nullptr);

    while(1) {
        usleep(500000);
    }    
    std::cout << "over ..." << std::endl;
}