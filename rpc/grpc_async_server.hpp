#ifndef __GRPC_ASYNC_SERVER_H__
#define __GRPC_ASYNC_SERVER_H__

#include <grpc++/grpc++.h>
#include "selectable_obj.hpp"

// a call_round_it is a calling process
// every grpc function/interface should inherit from this
class call_round_it
{
public:
	enum call_step_t
	{
		CREATE = 0,			//
		PROCESS = 1,		// calling
		FINISH = 2,			//
	};

	call_round_it() : m_step(CREATE)
	{}

	virtual ~call_round_it()
	{}

	void re_serve();

	virtual call_round_it *create_me() = 0;	// create a copy of me
	virtual int request_service() = 0;		// add this rpc call into listen
	virtual int do_work() = 0;				// bussiness code

	void process();

protected:
	call_step_t m_step;
};

class grpc_async_server_it : public selectable_object_it
{
public:
	grpc_async_server_it();
	virtual ~grpc_async_server_it();

	// register my service
	virtual int register_service(::grpc::ServerBuilder &builder) = 0;
	// start listen call requests
	virtual int listen_requests() = 0;

public:
	int start(const std::string & addr); //"0.0.0.0:50051"
	int stop();

	// called by some thread, then the call round will run in that thread
	virtual int select(int wait_ms);

protected:
	int start_grpc_server(const std::string & addr);

protected:
	bool m_running;
	std::unique_ptr<::grpc::ServerCompletionQueue> m_grpc_que_ptr;		// one server/port one que, used by all rpc interfaces.
	std::unique_ptr<::grpc::Server> m_grpc_server_ptr;
};

#endif
