#include <grpc/impl/codegen/gpr_types.h>
#include <grpc/support/time.h>
#include "grpc_async_client.hpp"
#include "logger.hpp"


namespace chr {

grpc_async_client_t::grpc_async_client_t(const std::string & addr)
: m_channel(::grpc::CreateChannel(addr, ::grpc::InsecureChannelCredentials()))
, m_addr(addr)
{
}

int grpc_async_client_t::select(int wait_ms)
{
	int proccessed = 0;

	void* tag;
	bool ok;

	// wait_ms should be 0 now, cause we have not hook the grpc APIs.
	// if wait_ms > 0, the os thread would be blocked, that's not what we want
	wait_ms = 0;	// FIXME

	gpr_timespec time;
	time.tv_sec = 0;
	time.tv_nsec = wait_ms * 1000000;
	//if (wait_ms > 0)	{
		time.clock_type = GPR_TIMESPAN;
	//}

	while (::grpc::CompletionQueue::GOT_EVENT == m_completion_que.AsyncNext(&tag, &ok, time) && ok)
	{
		proccessed++;
		client_call_it* call_ptr = static_cast<client_call_it*>(tag);
		if (call_ptr)
			call_ptr->on_rsp();

		if (proccessed > 10)
			break;
	}
	return proccessed;
}

bool grpc_async_client_t::ready()
{
	return m_channel.get()->GetState(true) == GRPC_CHANNEL_READY;
}

client_call_it * client_call_it::call()
{
	client_call_it * call_ptr = clone_me();
	if (call_ptr) {
		call_ptr->set_opt();
		call_ptr->call_impl();
	}
	return call_ptr;
}

void client_call_it::set_opt()
{
	if (m_timeout_seconds > 0) {
		m_context.set_deadline(std::chrono::system_clock::now() +
			std::chrono::seconds(m_timeout_seconds));
	}
}

}