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

class curl_req_t;
class curl_rsp_t;
class curl_stub_t : public selectable_object_it
{
public:    
    curl_stub_t();
    ~curl_stub_t();

    std::shared_ptr<curl_rsp_t> exec_curl(const std::string & url);
	virtual int select(int wait_ms);

private:
    typedef std::deque<std::shared_ptr<curl_req_t> > curl_req_que_t;
    typedef std::unordered_map<void *, std::shared_ptr<curl_req_t> > curl_req_map_t;

    size_t push_curl_req(std::shared_ptr<curl_req_t> req);
    bool add_todo_to_doing(); //if any req being executing, return true, else false
    void execute_all_async();
    void read_and_clean();
    

private:
    curl_req_que_t  m_curl_req_todo_que;       // reqs waiting to be executed
    curl_req_map_t  m_curl_req_doing_map;      // reqs being executed
	CURLM  *        m_multi_handle = nullptr;

    std::recursive_mutex    m_todo_que_lock;
    bool                    m_running = false;
    bool                    m_need_exit = false;

    // for time test
public:
    static std::time_t get_time_stamp();
    std::time_t     m_time_start;
    std::time_t     m_time_over;
};

#endif
