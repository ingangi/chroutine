/// \file curl_stub.hpp
/// 
/// inherited from selectable_object_it.
/// implmented the http client used in chroutine.
///
/// \author ingangi
/// \version 0.1.0
/// \date 2019-03-08

#ifndef CURL_STUB_H_
#define CURL_STUB_H_

#include <deque>
#include <memory>
#include <unordered_map>
#include "selectable_obj.hpp"
#include "curl_req.hpp"

namespace chr {

// A curl_stub_t is reused by all requests in a chroutine_thread_t 
class curl_stub_t : public selectable_object_it
{
    friend class engine_t;
public:    
    virtual ~curl_stub_t();    
	virtual int select(int wait_ms);
    
    // create and bind to a thread
	static selectable_object_sptr_t create(std::thread::id thread_id) {
		curl_stub_t *p_this = new curl_stub_t();
		selectable_object_sptr_t s_this = p_this->register_to_engin(thread_id);		
		if (s_this.get() == nullptr) {
			delete p_this;
		} 
		return s_this;
	}

private:
    curl_stub_t();
	typedef struct {}curl_call_wait_t;
    typedef std::deque<std::shared_ptr<curl_req_t> > curl_req_que_t;
    typedef std::unordered_map<void *, std::shared_ptr<curl_req_t> > curl_req_map_t;

    size_t push_curl_req(std::shared_ptr<curl_req_t> req);
    bool add_todo_to_doing(); //if any req being executing, return true, else false
    void execute_all_async();
    void read_and_clean();
    
    std::shared_ptr<curl_rsp_t> exec_curl(const std::string & url
        , int connect_timeout
        , int timeout
        , data_slot_func_t w_func
        , void *w_func_handler);

private:
    curl_req_que_t  m_curl_req_todo_que;       // reqs waiting to be executed
    curl_req_map_t  m_curl_req_doing_map;      // reqs being executed
	CURLM  *        m_multi_handle = nullptr;

    bool                    m_running = false;
    bool                    m_need_exit = false;
};

}
#endif
