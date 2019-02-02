#include <grpc/impl/codegen/gpr_types.h>
#include <grpc/support/time.h>
#include "grpc_async_server.hpp"


void call_round_it::re_serve()
{
	call_round_it * this_ptr = create_me();
	if (this_ptr)
		this_ptr->process();
}

void call_round_it::process()
{
	if (m_step == CREATE) {
		m_step = PROCESS;
		request_service();
	} else if (m_step == PROCESS) {
		m_step = FINISH;
		// in case of new request come during process, we make a new me to serve
		re_serve();
		do_work();
	} else {
		if (m_step != FINISH) {
			//SPDLOG(ERR, "wrong state {}, delete it anyway", m_step);
		}
		delete this;
	}
}

int grpc_async_server_it::select(int wait_ms)
{
	int proccessed = 0;
	if (!m_running)
		return proccessed;

	gpr_timespec time;
	time.tv_sec = 0;
	time.tv_nsec = wait_ms * 1000000;
	//if (wait_ms > 0) {
		time.clock_type = GPR_TIMESPAN;
	//}

	void* tag;
	bool ok;
	while (::grpc::CompletionQueue::GOT_EVENT == m_grpc_que_ptr.get()->AsyncNext(&tag, &ok, time) && ok) {
		proccessed++;
		call_round_it* round_ptr = static_cast<call_round_it*>(tag);
		if (round_ptr) {
			round_ptr->process();
		}

		// yield when too busy
		if (proccessed > 100)
			break;
	}
	return proccessed;
}

grpc_async_server_it::grpc_async_server_it() : m_running(false)
{
}

grpc_async_server_it::~grpc_async_server_it()
{
	stop();
}

int grpc_async_server_it::start(const std::string & addr)
{
	if (m_running) {
		//SPDLOG(CRITICAL, "start grpc_async_server_it({}) failed, already running!", addr);
		return 0;
	}

	// check if this thread can reigster to engin
	if (register_to_engin() != 0) {
		std::cout << __FUNCTION__ << " error: cant register to engin\n";
		return -1;
	}

	//SPDLOG(CRITICAL, "start grpc_async_server_it({}) thread!", addr);

	if (0 != start_grpc_server(addr))
		return -1;

	listen_requests();

	m_running = true;
	return 0;
}

int grpc_async_server_it::stop()
{
	m_running = false;
	m_grpc_server_ptr->Shutdown();
	m_grpc_que_ptr->Shutdown();	
	return 0;
}

int grpc_async_server_it::start_grpc_server(const std::string & addr)
{
	::grpc::ServerBuilder builder;
	builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
	register_service(builder);
	m_grpc_que_ptr = builder.AddCompletionQueue();
	m_grpc_server_ptr = builder.BuildAndStart();
	return 0;
}