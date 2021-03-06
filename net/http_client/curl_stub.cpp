#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <unistd.h>

#include "curl_req.hpp"
#include "curl_stub.hpp"
#include "engine.hpp"

namespace chr {

const long MAX_CONCURRENT_TRANS_IN_CURLMULTI = 1000;


curl_stub_t::curl_stub_t()
{
    m_multi_handle = curl_multi_init();
    if (m_multi_handle == nullptr)  {
        SPDLOG(CRITICAL, "curl_multi_init error!");
        exit(1);
        return;
    }

	curl_multi_setopt(m_multi_handle, CURLMOPT_MAXCONNECTS, MAX_CONCURRENT_TRANS_IN_CURLMULTI);
}

curl_stub_t::~curl_stub_t()
{
    curl_multi_cleanup(m_multi_handle);
}


std::shared_ptr<curl_rsp_t> curl_stub_t::exec_curl(const std::string & url
    , int connect_timeout
    , int timeout
    , data_slot_func_t w_func
    , void *w_func_handler)
{
    chroutine_id_t chroutine_id = ENGIN.get_current_chroutine_id();
    if (chroutine_id == INVALID_ID) {
        SPDLOG(ERROR, "curl_stub_t::exec_curl error: can't get current_chroutine_id!");
        return nullptr;
    }

    auto req = curl_req_t::new_curl_req(0);
    curl_req_t *p_req = req.get();
    if (p_req == nullptr) {
        return nullptr;
    }

    p_req->set_my_chroutine_id(chroutine_id);
    p_req->make_default_opts();
    p_req->set_url(url);
    if (connect_timeout > 0 && connect_timeout != CURL_CON_TIME_OUT) {
        p_req->set_connect_timeout(connect_timeout);
    }
    if (timeout > 0 && timeout != CURL_TIME_OUT) {
        p_req->set_timeout(timeout);
    }
    if (w_func && w_func_handler) {
        p_req->set_data_slot(w_func, w_func_handler);
    }

    push_curl_req(req);

    // wait req to be done
    if (timeout > 0) {
        reporter_base_t * rpt = ENGIN.create_son_chroutine([](void *d) {
            while (1) {SLEEP(10000);}
        }, reporter_t<curl_call_wait_t>::create(), timeout);
        
        if (rpt) {
            SPDLOG(DEBUG, "curl_call_wait_t, call result:{}", rpt->get_result());
        }
    }

    // get rsp from req
    return std::shared_ptr<curl_rsp_t>(new curl_rsp_t(std::move(p_req->rsp())));
}

int curl_stub_t::select(int wait_ms)
{
    if (add_todo_to_doing()) {
        execute_all_async();
        read_and_clean();
    }
    return m_curl_req_doing_map.size();
}

size_t curl_stub_t::push_curl_req(std::shared_ptr<curl_req_t> req)
{
    m_curl_req_todo_que.push_back(req);
    return m_curl_req_todo_que.size();
}

bool curl_stub_t::add_todo_to_doing()
{    
    while (m_curl_req_doing_map.size() < MAX_CONCURRENT_TRANS_IN_CURLMULTI 
            && !m_curl_req_todo_que.empty()) {

        auto ptr_of_req = m_curl_req_todo_que.front();
        curl_req_t* req = ptr_of_req.get();
        m_curl_req_todo_que.pop_front();
        if (req && req->get_curl_handler()) {
            req->attach_multi_handler(m_multi_handle);
            m_curl_req_doing_map[req->get_curl_handler()] = ptr_of_req;
        }
    }
    return !m_curl_req_doing_map.empty();
}

const int SELECT_TIMEOUT_TIMES = 5;
const int SELECT_TIMES = 10;
static const int SELECT_TIMEOUT = 10;
void curl_stub_t::execute_all_async()
{
	int           nCountOfEasyHandlesRun = -1;
	int           numfds = 0;
	int           nSelectTimeoutTimes = 0;
	int           nSelectTimes = 0;
    
	curl_multi_perform(m_multi_handle, &nCountOfEasyHandlesRun);   
	while (nCountOfEasyHandlesRun > 0
		&& nSelectTimes < SELECT_TIMES
		&& nSelectTimeoutTimes < SELECT_TIMEOUT_TIMES) {

		if (curl_multi_wait(m_multi_handle, NULL, 0, SELECT_TIMEOUT, &numfds) != CURLM_OK) {
			return;
		}
		nSelectTimes++;
		if (!numfds) {
			nSelectTimeoutTimes++;
		} else {
			nSelectTimeoutTimes = 0;
			curl_multi_perform(m_multi_handle, &nCountOfEasyHandlesRun);
		}
	}    
}

void curl_stub_t::read_and_clean()
{
	CURLMsg * msg = nullptr;
	int msg_count = 0;
	while (msg = curl_multi_info_read(m_multi_handle, &msg_count), msg)	{
		if (msg->msg == CURLMSG_DONE) {
            long rsp_code;
            long data_result;
            
			CURL *easy_handle = msg->easy_handle;
			curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE,  &rsp_code);
            data_result = msg->data.result;		

            auto iter = m_curl_req_doing_map.find(easy_handle);
            if (iter != m_curl_req_doing_map.end()) {
                curl_req_t *req_ptr = iter->second.get();
                if (req_ptr) {
                    req_ptr->detach_multi_handler(m_multi_handle);
                    req_ptr->on_rsp(rsp_code, data_result);
                }
                m_curl_req_doing_map.erase(iter);
            } else {
                SPDLOG(ERROR, "cant find curl_req_t in m_curl_req_doing_map");
            }
		}
	}
}

}