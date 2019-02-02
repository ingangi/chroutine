#include <grpc/impl/codegen/gpr_types.h>
#include <grpc/support/time.h>
#include "grpc_async_client.hpp"

grpc_async_client_t::grpc_async_client_t(const std::string & addr)
: m_channel(::grpc::CreateChannel(addr, ::grpc::InsecureChannelCredentials()))
, m_addr(addr)
{
	std::cout << addr << "-> channel = " << m_channel.get() << std::endl;
	if (register_to_engin() != 0) {
		std::cout << __FUNCTION__ << " error: cant register to engin\n";
		exit(-1);
	}
}

int grpc_async_client_t::select(int wait_ms)
{
	int proccessed = 0;

	void* tag;
	bool ok;

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
		{
			call_ptr->on_rsp();
			// FIXME
			//delete call_ptr;	// todo: find a better way to manage call_ptr
		}

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
	if (call_ptr)
	{
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
