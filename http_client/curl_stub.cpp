#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <unistd.h>

#include "curl_req.h"
#include "curl_stub.h"

const long MAX_CONCURRENT_TRANS_IN_CURLMULTI = 1000;


curl_stub_t::curl_stub_t()
{
    curl_global_init(CURL_GLOBAL_ALL);
    m_multi_handle = curl_multi_init();
    if (m_multi_handle == nullptr)  {
        std::cout << "curl_multi_init error!" << std::endl;
        exit(1);
        return;
    }

	curl_multi_setopt(m_multi_handle, CURLMOPT_MAXCONNECTS, MAX_CONCURRENT_TRANS_IN_CURLMULTI);
}

curl_stub_t::~curl_stub_t()
{
    curl_multi_cleanup(m_multi_handle);
    curl_global_cleanup();
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
    // do we need limit todo-list size ?
    std::lock_guard<std::recursive_mutex> lock(m_todo_que_lock);
    m_curl_req_todo_que.push_back(req);
    return m_curl_req_todo_que.size();
}

bool curl_stub_t::add_todo_to_doing()
{
    //std::cout << "[trace] add_todo_to_doing" << std::endl;
    // add to multi handler
    std::lock_guard<std::recursive_mutex> lock(m_todo_que_lock);
    
    while (m_curl_req_doing_map.size() < MAX_CONCURRENT_TRANS_IN_CURLMULTI  // m_curl_req_doing_map.size() is O(1)
            && !m_curl_req_todo_que.empty()) {

        auto ptr_of_req = m_curl_req_todo_que.front();
        curl_req_t* req = ptr_of_req.get();
        m_curl_req_todo_que.pop_front();
        if (req && req->get_curl_handler()) {
            req->attach_multi_handler(m_multi_handle);
            m_curl_req_doing_map[req->get_curl_handler()] = ptr_of_req;
        }
    }
    //std::cout << "[trace] add_todo_to_doing over" << std::endl;
    return !m_curl_req_doing_map.empty();
}

const int SELECT_TIMEOUT_TIMES = 5;
const int SELECT_TIMES = 10;
static const int SELECT_TIMEOUT = 10;
void curl_stub_t::execute_all_async()
{
    std::cout << "[trace] execute_all_async" << std::endl;

	int           nCountOfEasyHandlesRun = -1;
	int           numfds = 0;
	int           nSelectTimeoutTimes = 0;
	int           nSelectTimes = 0;

    std::cout << "[trace] curl_multi_perform while begin" << std::endl;
    
	curl_multi_perform(m_multi_handle, &nCountOfEasyHandlesRun);   
	while (nCountOfEasyHandlesRun > 0
		&& nSelectTimes < SELECT_TIMES
		&& nSelectTimeoutTimes < SELECT_TIMEOUT_TIMES) {

		if (curl_multi_wait(m_multi_handle, NULL, 0, SELECT_TIMEOUT, &numfds) != CURLM_OK) {
			std::cout << ("HttpAgent::PerformTransfer, curl_multi_wait fail\n");
			return;
		}
		nSelectTimes++;
		if (!numfds) {
			nSelectTimeoutTimes++; /* count number of repeated zero numfds */
		} else {
			nSelectTimeoutTimes = 0;
			curl_multi_perform(m_multi_handle, &nCountOfEasyHandlesRun);
			if (nSelectTimes % 10 == 0) //reduce log;
				std::cout << "HttpAgent::PerformTransfer after one while, nCountOfEasyHandlesRun=" << nCountOfEasyHandlesRun << ",nSelectTimes=" << nSelectTimes << ", nSelectTimeoutTimes=" << nSelectTimeoutTimes << std::endl;
		}
	}
    
    std::cout << "[trace] execute_all_async over" << std::endl;
}

void curl_stub_t::read_and_clean()
{
    std::cout << "[trace] read_and_clean" << std::endl;

	CURLMsg * msg = nullptr;
    //  CURLMsg define: https://curl.haxx.se/libcurl/c/curl_multi_info_read.html
    //  struct CURLMsg {
    //    CURLMSG msg;       /* what this message means */
    //    CURL *easy_handle; /* the handle it concerns */
    //    union {
    //      void *whatever;    /* message-specific data */
    //      CURLcode result;   /* return code for transfer */
    //    } data;
    //  };
    // When msg is CURLMSG_DONE, the message identifies a transfer that is done, 
    // and then result contains the return code for the easy handle that just completed.

    static int fin_count = 0; //for test

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
                    req_ptr->rsp().set_rsp_code(rsp_code);
                    req_ptr->rsp().set_curl_code(data_result);
                }
                m_curl_req_doing_map.erase(iter);

                // todo: post resut to business thread
                // time_t now = time(NULL);
			    // std::cout << "[" << now << "] req:" << req_ptr->req_id() 
                //     << " rsp_code:" << rsp_code 
                //     << ", data_result:" << data_result 
                //     << " todo.size " << m_curl_req_todo_que.size() 
                //     << ", doing.size " << m_curl_req_doing_map.size() 
                //     << "|" << get_time_stamp() - req_ptr->m_tm_start
                //     << std::endl;
                
                std::cout << "HTEST: id[" << req_ptr->req_id() << "], cost(" << get_time_stamp() - req_ptr->m_tm_start << "ms),"
                      << " rsp_code(" << rsp_code 
                    << "), data_result(" << data_result  << ")" << std::endl;

                fin_count++;
                if (m_curl_req_todo_que.empty() && m_curl_req_doing_map.empty()) {
                    m_time_over = get_time_stamp();
                    std::cout << "[trace] finish: " << fin_count 
                        << " cost time: " << m_time_over - m_time_start 
                        << std::endl;
                }
            } else {
                std::cout << "cant find curl_req_t in m_curl_req_doing_map\n";
            }
		}
	}
    std::cout << "[trace] read_and_clean over" << std::endl;
}

std::time_t curl_stub_t::get_time_stamp()
{
    std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
}