#include "test_server.hpp"
#include "logger.hpp"
#include "engine.hpp"


template <> int service_api_t<rpcpb::Test, rpcpb::TestReq, rpcpb::TestRsp>::request_service()
{
	m_service_ptr->RequestHowAreYou(&m_ctx, &m_req_msg, &m_responser, m_que_ptr, m_que_ptr, this);
	return 0;
}

template <> int service_api_t<rpcpb::Test, rpcpb::TestReq, rpcpb::TestRsp>::do_work()
{
	assert(ENGIN.get_current_chroutine_id() != chr::INVALID_ID);
    SPDLOG(INFO, "{} HowAreYou get req, in chroutine: {}", __FUNCTION__, ENGIN.get_current_chroutine_id());

	SLEEP(2000);	//fake processing

    SPDLOG(INFO, "{}: process over, response now", __FUNCTION__);
	m_rsp_msg.set_rsp("Fine thank you, and you?!");
	::grpc::Status status;
	m_responser.Finish(m_rsp_msg, status, this);
	return 0;
}


int test_rpc_server::register_service(::grpc::ServerBuilder &builder)
{
	builder.RegisterService(&m_service_Test);
	return 0;
}

int test_rpc_server::listen_requests()
{
	service_api_t<rpcpb::Test, rpcpb::TestReq, rpcpb::TestRsp>(&m_service_Test, m_grpc_que_ptr.get()).re_serve();
	return 0;
}
