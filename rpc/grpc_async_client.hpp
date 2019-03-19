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
	virtual ~grpc_async_client_t(){}
	const std::string &addr(){ return m_addr; }
	
	static selectable_object_sptr_t create(const std::string & addr) {
		grpc_async_client_t *p_this = new grpc_async_client_t(addr);
		selectable_object_sptr_t s_this = p_this->register_to_engin();		
		if (s_this.get() == nullptr) {
			delete p_this;
		} 
		return s_this;
	}
	
private:
	grpc_async_client_t(const std::string & addr);

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

	client_call_it * call();
	virtual int on_rsp() = 0;

	// should be called before `call()`
	void set_timeout(int seconds) {
		m_timeout_seconds = seconds;
	}

protected:
	virtual client_call_it *clone_me() = 0;
	virtual int call_impl() = 0;
	virtual void set_opt();

protected:
	::grpc::ClientContext m_context;
	::grpc::Status m_status;
	grpc_async_client_t *m_client_ptr = nullptr;
	int		m_timeout_seconds = 0;	// 0 -> no time limite
};

#endif