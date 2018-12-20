#ifndef TEST_SERVER_H
#define TEST_SERVER_H

// `Test` service is defined in proto_def/test.proto

#include "test.grpc.pb.h"
#include "grpc_async_server.hpp"

// you have to implement every rpc-interfaces you defined in proto file by write
// a class for each of them.  Fuck!
// recommended class name format: "[SERVICE]_[INTERFACE]"
// for example (see in proto_def/test.proto):
// service Test {
//     rpc HowAreYou(TestReq) returns (TestRsp) {}
// }
class Test_HowAreYou final : public call_round_it
{
  public:
	Test_HowAreYou(::rpcpb::Test::AsyncService *service, ::grpc::ServerCompletionQueue *cq)
		: m_service_ptr(service)
		, m_que_ptr(cq)
		, m_responser(&m_ctx)
	{}

	call_round_it *create_me() {
		return new Test_HowAreYou(m_service_ptr, m_que_ptr);
	}

	// add to listen
	int request_service() {
		m_service_ptr->RequestHowAreYou(&m_ctx, &m_req_msg, &m_responser, m_que_ptr, m_que_ptr, this);
		return 0;
	}

	// called when client requests come
	int do_work();

  public:
	::rpcpb::Test::AsyncService *m_service_ptr;
	::grpc::ServerCompletionQueue *m_que_ptr;
	::grpc::ServerContext m_ctx;
	rpcpb::TestReq m_req_msg;
	rpcpb::TestRsp m_rsp_msg;
	::grpc::ServerAsyncResponseWriter<rpcpb::TestRsp> m_responser;
};

// it is your server, it carries the `Test` service and some other services if needed.
// you have to pick a ip&port for your server to run on by calling the `start` function.
class test_rpc_server : public grpc_async_server_it
{
  public:
	test_rpc_server(){}
	~test_rpc_server(){}
	int register_service(::grpc::ServerBuilder &builder);
	int listen_requests();

  private:
	::rpcpb::Test::AsyncService m_service_Test; // `Test` service
};

#endif
