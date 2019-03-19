#include "test_server.hpp"
#include "engine.hpp"
#include <unistd.h>


int main(int argc, char **argv)
{   
    ENGINE_INIT(1);
    ENGIN.create_chroutine([](void *){
        test_rpc_server *server = dynamic_cast<test_rpc_server *>(test_rpc_server::create().get());
        if (server == nullptr || server->start("0.0.0.0:50061") != 0) {
            std::cout << "test_rpc_server start failed\n";
        }
        std::cout << "test_rpc_server is running.\n";
    }, nullptr);
    
    ENGIN.run(); 
    std::cout << "over ..." << std::endl;
}