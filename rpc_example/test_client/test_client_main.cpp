#include "test_client.hpp"
#include "engine.hpp"
#include <unistd.h>


int main(int argc, char **argv)
{   
    ENGINE_INIT(1);
    ENGIN.create_chroutine([](void *){
        grpc_async_client_t *client = new grpc_async_client_t("127.0.0.1:50061");
        while (1) {
            call_Test_HowAreYou caller(client);
            rpcpb::TestRsp rsp;
            ::grpc::StatusCode code = caller.call_sync(rsp);
            std::cout << "call_Test_HowAreYou code=" << code << ", rsp:" << rsp.rsp() << std::endl;
            SLEEP(5000);
        }
        delete client;
    }, nullptr);

    ENGIN.run();  
    std::cout << "over ..." << std::endl;
}