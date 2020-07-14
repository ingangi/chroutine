/// \file grpc_async_server.hpp
/// 
/// inherited from selectable_object_it.
/// implmented the async rpc server based on grpc.
///
/// \author ingangi
/// \version 0.1.0
/// \date 2018-12-24

#ifndef __GRPC_ASYNC_SERVER_H__
#define __GRPC_ASYNC_SERVER_H__

#include <grpc++/grpc++.h>
#include "selectable_obj.hpp"

namespace chr {

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

// One `server` corresponds to one ip&port, can carry multiple `services`
class grpc_async_server_it : public selectable_object_it
{
public:
	grpc_async_server_it();
	virtual ~grpc_async_server_it();

	// register all services carried by this server
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

}

template<typename service_type_t, typename req_type_t, typename rsp_type_t>
class service_api_t final : public chr::call_round_it
{
  public:
    service_api_t(typename service_type_t::AsyncService *service, ::grpc::ServerCompletionQueue *cq)
        : m_service_ptr(service)
        , m_que_ptr(cq)
        , m_responser(&m_ctx)
    {}

    chr::call_round_it *create_me() {
        return new service_api_t<service_type_t,req_type_t,rsp_type_t>(m_service_ptr, m_que_ptr);
    }

    // add to listen
    int request_service() {
        return 0;
    }

    // called when client requests come
    int do_work() {
		return 0;
	}

  public:
    typename service_type_t::AsyncService *m_service_ptr;
    ::grpc::ServerCompletionQueue *m_que_ptr;
    ::grpc::ServerContext m_ctx;
    req_type_t m_req_msg;
    rsp_type_t m_rsp_msg;
    ::grpc::ServerAsyncResponseWriter<rsp_type_t> m_responser;
};
#endif
