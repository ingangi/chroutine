#ifndef TEST_CLIENT_H
#define TEST_CLIENT_H

// `Test` service is defined in proto_def/test.proto

#include "test.grpc.pb.h"
#include "grpc_sync_client_for_chroutine.hpp"

// can not reuse!  FIXME
class call_Test_HowAreYou final : public client_sync_call_t<rpcpb::TestRsp>
{
public:
	call_Test_HowAreYou(grpc_async_client_t *client);
	call_Test_HowAreYou(call_Test_HowAreYou &other);

private:
	client_call_it *clone_me();
	int call_impl();

public:
	std::unique_ptr<rpcpb::Test::Stub> m_stub;
	rpcpb::TestReq m_req;
	std::unique_ptr<::grpc::ClientAsyncResponseReader<rpcpb::TestRsp> > m_rsp_reader;
};

#endif