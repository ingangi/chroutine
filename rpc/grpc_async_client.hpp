/// \file grpc_async_client.hpp
/// 
/// inherited from selectable_object_it.
/// implmented the async rpc client based on grpc.
///
/// \author ingangi
/// \version 0.1.0
/// \date 2018-12-24

#ifndef __GRPC_ASYNC_CLIENT_H__
#define __GRPC_ASYNC_CLIENT_H__

#include <grpc++/grpc++.h>
#include "selectable_obj.hpp"

class grpc_async_client_t : public selectable_object_it
{
public:
	grpc_async_client_t(const std::string & addr);
	virtual ~grpc_async_client_t(){}

	const std::string &addr(){ return m_addr; }

public:
	virtual int select(int wait_ms);

	// whether the grpc channel is ready
	bool ready();
	
public:
	::grpc::CompletionQueue m_completion_que;
	std::shared_ptr<::grpc::Channel> m_channel;
	std::string m_addr;
};

// Implement the invocation and reply processing of an rpc-interface
class client_call_it
{
public:
	client_call_it(grpc_async_client_t *client_ptr) : m_client_ptr(client_ptr)
	{}
	virtual ~client_call_it()
	{}

	int call();
	virtual int on_rsp() = 0;

protected:
	virtual client_call_it *clone_me() = 0;
	virtual int call_impl() = 0;
	virtual void set_timeout();

protected:
	::grpc::ClientContext m_context;
	::grpc::Status m_status;
	grpc_async_client_t *m_client_ptr = nullptr;
};


#endif