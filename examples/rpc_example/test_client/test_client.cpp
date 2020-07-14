#include "test_client.hpp"

template <> 
int call_service_api_t<rpcpb::Test, rpcpb::TestReq, rpcpb::TestRsp>::call_impl()
{
    m_stub = rpcpb::Test::NewStub(m_client_ptr->m_channel);
    m_rsp_reader = m_stub->AsyncHowAreYou(&m_context, m_req, &m_client_ptr->m_completion_que);
    m_rsp_reader->Finish(&m_result, &m_status, (void*)this);
    return 0;
}