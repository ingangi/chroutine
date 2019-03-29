/// \file grpc_sync_client_for_chroutine.hpp
/// 
/// implmented the sync rpc client based on grpc which can use in chroutine.
///
/// \author ingangi
/// \version 0.1.0
/// \date 2019-02-02

#ifndef __GRPC_SYNC_CLIENT_FOR_CHROUTINE_H__
#define __GRPC_SYNC_CLIENT_FOR_CHROUTINE_H__

#include "grpc_async_client.hpp"
#include "engine.hpp"

namespace chr {

template<typename RESULT_T>
class client_sync_call_t : public client_call_it
{
public:
	client_sync_call_t(grpc_async_client_t *client_ptr) : client_call_it(client_ptr)
	{
		set_timeout(3);
	}


	client_sync_call_t(client_sync_call_t &other) : client_call_it(other.m_client_ptr) {
		m_my_chroutine = other.m_my_chroutine;
		m_chroutine_timeout_ms = other.m_chroutine_timeout_ms;
		m_result = other.m_result;
	}

	virtual ~client_sync_call_t()
	{}

	::grpc::StatusCode call_sync(RESULT_T &result) {
		client_sync_call_t<RESULT_T> *call_ptr = dynamic_cast<client_sync_call_t<RESULT_T> *>(call());
		if (call_ptr) {
			call_ptr->wait_call();
			result = call_ptr->m_result;
			::grpc::StatusCode code = call_ptr->m_status.error_code();
			delete call_ptr;
			return code;
		}
		return ::grpc::StatusCode::UNKNOWN;
	}
	
	virtual int on_rsp() {
		// awake calling chroutine
		if (m_my_chroutine != INVALID_ID) {
			ENGIN.awake_chroutine(m_my_chroutine);
		}
        return 0;
	}
	
	void set_timeout(int seconds) {
		m_timeout_seconds = seconds;
		if (seconds > 0) {
			m_chroutine_timeout_ms = seconds * 1000 + 500;
		} else {
			m_chroutine_timeout_ms = 60*1000;	// max 1 minute
		}
	}

protected:
	typedef struct {

	}rpc_call_wait_t;

	virtual int wait_call() {
		// current chroutine goes to wait
		m_my_chroutine = ENGIN.get_current_chroutine_id();
		if (m_my_chroutine != INVALID_ID && m_chroutine_timeout_ms > 0) {
            reporter_base_t * rpt = ENGIN.create_son_chroutine([](void *d) {
				while (1) {SLEEP(10000);}
			}, reporter_t<rpc_call_wait_t>::create(), m_chroutine_timeout_ms);
			
			if (rpt) {
				LOG << "wait_call, call result:" << rpt->get_result() 
				<< ", status.ok:" << m_status.ok() << std::endl;
			}
		}
		// 
		return 0;
	}
public:
	RESULT_T m_result;
private:
	chroutine_id_t m_my_chroutine = INVALID_ID;
	int m_chroutine_timeout_ms = 0;
};

}
#endif
