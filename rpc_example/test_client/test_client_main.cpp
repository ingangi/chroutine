#include "test_client.hpp"
#include "engine.hpp"
#include <unistd.h>


int main(int argc, char **argv)
{   
    ENGINE_INIT(1);
    ENGIN.create_chroutine([](void *){
        grpc_async_client_t *client = dynamic_cast<grpc_async_client_t *>(grpc_async_client_t::create("127.0.0.1:50061").get());
        if (client == nullptr) {
            std::cout << "grpc_async_client_t::create failed\n";
            return;
        }

        int i = 0;
        while (client) {
            call_Test_HowAreYou caller(client);
            rpcpb::TestRsp rsp;
            ::grpc::StatusCode code = caller.call_sync(rsp);
            std::cout << "call_Test_HowAreYou code=" << code << ", rsp:" << rsp.rsp() << std::endl;

            if (++i >= 3) {
                std::cout << "grpc_async_client_t will unregister now.\n";
                client->unregister_from_engin();
                client = nullptr;
            }
            SLEEP(3000);
        }
        
    }, nullptr);

    ENGIN.run();  
    std::cout << "over ..." << std::endl;
}