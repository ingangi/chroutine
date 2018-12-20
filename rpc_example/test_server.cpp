#include "test_server.hpp"

int Test_HowAreYou::do_work() 
{
    std::cout << __FUNCTION__ << " HowAreYou get req\n";    
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
	Test_HowAreYou(&m_service_Test, m_grpc_que_ptr.get()).re_serve();
	return 0;
}
