#include "test_client.hpp"

call_Test_HowAreYou::call_Test_HowAreYou(grpc_async_client_t *client) : client_sync_call_t(client)
{
}

call_Test_HowAreYou::call_Test_HowAreYou(call_Test_HowAreYou &other) : client_sync_call_t(other)
{
	m_req = other.m_req;
}

int call_Test_HowAreYou::call_impl()
{
	m_stub = rpcpb::Test::NewStub(m_client_ptr->m_channel);
	m_rsp_reader = m_stub->AsyncHowAreYou(&m_context, m_req, &m_client_ptr->m_completion_que);
	m_rsp_reader->Finish(&m_result, &m_status, (void*)this);
	return 0;
}

client_call_it * call_Test_HowAreYou::clone_me()
{
	return new call_Test_HowAreYou(*this);
}