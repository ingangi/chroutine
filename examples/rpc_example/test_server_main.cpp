#include "test_server.hpp"
#include "engine.hpp"
#include <unistd.h>

using namespace chr;

int main(int argc, char **argv)
{   
    ENGINE_INIT(1);
    ENGIN.create_chroutine([](void *){
        test_rpc_server *server = dynamic_cast<test_rpc_server *>(test_rpc_server::create().get());
        if (server == nullptr || server->start("0.0.0.0:50061") != 0) {
            SPDLOG(INFO, "test_rpc_server start failed");
        }
        SPDLOG(INFO, "test_rpc_server is running.");
    }, nullptr);
    
    ENGIN.run();
}