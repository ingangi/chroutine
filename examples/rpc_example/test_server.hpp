#ifndef TEST_SERVER_H
#define TEST_SERVER_H

// `Test` service is defined in proto_def/test.proto

#include "test.grpc.pb.h"
#include "grpc_async_server.hpp"

// service Test {
//     rpc HowAreYou(TestReq) returns (TestRsp) {}
// }

// for API: Test::HowAreYou
template <> int service_api_t<rpcpb::Test, rpcpb::TestReq, rpcpb::TestRsp>::request_service();
template <> int service_api_t<rpcpb::Test, rpcpb::TestReq, rpcpb::TestRsp>::do_work();

// it is your server, it carries the `Test` service and some other services if needed.
// you have to pick a ip&port for your server to run on by calling the `start` function.
class test_rpc_server : public chr::grpc_async_server_it
{
public:
    static chr::selectable_object_sptr_t create() {
        test_rpc_server *p_this = new test_rpc_server();
        chr::selectable_object_sptr_t s_this = p_this->register_to_engin();		
        if (s_this.get() == nullptr) {
            delete p_this;
        } 
        return s_this;
    }
    ~test_rpc_server(){}
    
    int register_service(::grpc::ServerBuilder &builder);
    int listen_requests();

private:
    test_rpc_server(){}
    
private:
    ::rpcpb::Test::AsyncService m_service_Test; // `Test` service
};

#endif
