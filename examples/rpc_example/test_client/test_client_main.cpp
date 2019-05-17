#include "test_client.hpp"
#include "engine.hpp"
#include <unistd.h>

using namespace chr;

int main(int argc, char **argv)
{   
    ENGINE_INIT(1);

    ENGIN.create_chroutine([](void *){
        grpc_async_client_t *client = dynamic_cast<grpc_async_client_t *>(grpc_async_client_t::create("127.0.0.1:50061").get());
        if (client == nullptr) {
            SPDLOG(INFO, "grpc_async_client_t::create failed");
            return;
        }

        SPDLOG(INFO, "grpc_async_client_t {} created, is it ready? {}", client, client->ready());

        int i = 0;
        call_Test_HowAreYou caller(client);
        rpcpb::TestRsp rsp;

        while (client) {            
            // you can wait for the client connected before do the call.
            // or you can just call it without check the ready state, then the StatusCode will return 14 on connection not ready.
            // if (!client->ready()) {
            //     SPDLOG(INFO, "waiting for client tobe ready");
            //     SLEEP(20);
            //     continue;
            // }

            SPDLOG(INFO, "try to call call_Test_HowAreYou the {}th time, is it ready? {}", i, client->ready());
            ::grpc::StatusCode code = caller.call_sync(rsp);
            SPDLOG(INFO, "call_Test_HowAreYou code={} , rsp:{}. clinet ready? {} ", code, rsp.rsp(), client->ready());

            // how we destroy a client
            if (++i >= 10) {
                SPDLOG(INFO, "grpc_async_client_t will unregister now.");
                client->unregister_from_engin();
                client = nullptr;
            }
            SLEEP(1000);
        }
        
    }, nullptr);

    ENGIN.run();  
}