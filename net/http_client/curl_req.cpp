
#include <iostream>
#include <cstring>
#include "curl_req.hpp"
#include "curl_stub.hpp"
#include "engine.hpp"

namespace chr {

curl_req_t::curl_req_t(unsigned int req_id) : m_req_id(req_id)
{
    m_curl_ptr = curl_easy_init();

    if (m_curl_ptr) {
	    curl_easy_setopt(m_curl_ptr, CURLOPT_HTTPGET, 1);
    }
}

std::shared_ptr<curl_req_t> curl_req_t::new_curl_req(unsigned int req_id)
{
    std::shared_ptr<curl_req_t> t(new curl_req_t(req_id));
    return t;
}

curl_req_t::~curl_req_t()
{
    curl_easy_cleanup(m_curl_ptr);
    if (m_post_buf) {
        delete [] m_post_buf;
    }
}

CURL *curl_req_t::get_curl_handler()
{
    return m_curl_ptr;
}

CURLcode curl_req_t::set_url(const std::string & str_url)
{
    if (!m_curl_ptr)
        return CURLE_FAILED_INIT;

    return curl_easy_setopt(m_curl_ptr, CURLOPT_URL, str_url.c_str());
}

CURLcode curl_req_t::set_connect_timeout(int timeout_ms)
{
    if (!m_curl_ptr)
        return CURLE_FAILED_INIT;

    return curl_easy_setopt(m_curl_ptr, CURLOPT_CONNECTTIMEOUT_MS, timeout_ms);
}

CURLcode curl_req_t::set_timeout(int timeout_ms)
{
    if (!m_curl_ptr)
        return CURLE_FAILED_INIT;

    return curl_easy_setopt(m_curl_ptr, CURLOPT_TIMEOUT_MS, timeout_ms);
}

CURLcode curl_req_t::set_data_slot(data_slot_func_t func, void *func_handler)
{
    if (!m_curl_ptr)
        return CURLE_FAILED_INIT;

    curl_easy_setopt(m_curl_ptr, CURLOPT_WRITEDATA, func_handler);
    return curl_easy_setopt(m_curl_ptr, CURLOPT_WRITEFUNCTION, func);
}

void curl_req_t::set_post_data(uint8_t *data, uint32_t len)
{
    if (data == nullptr || len == 0) {
        return;
    }

    m_type = EN_CURL_TYPE_POST;
    if (m_post_buf) {
        delete [] m_post_buf;
    }
    m_post_buf_len = len;
    m_post_buf = new uint8_t[m_post_buf_len];
    std::memcpy(m_post_buf, data, m_post_buf_len);    
    curl_easy_setopt(m_curl_ptr, CURLOPT_POST, 1);
    curl_easy_setopt(m_curl_ptr, CURLOPT_POSTFIELDSIZE, m_post_buf_len);
    curl_easy_setopt(m_curl_ptr, CURLOPT_POSTFIELDS, m_post_buf);
}

CURLcode curl_req_t::execute_sync()
{
    CURLcode ret = CURLE_FAILED_INIT;
    if (!m_curl_ptr) {
        return ret;
    }
    
    return curl_easy_perform(m_curl_ptr);
}

int curl_req_t::attach_multi_handler(CURLM *handler)
{
    return curl_multi_add_handle(handler, m_curl_ptr);
}

int curl_req_t::detach_multi_handler(CURLM *handler)
{
    return curl_multi_remove_handle(handler, m_curl_ptr);
}

void curl_req_t::make_default_opts()
{
    set_connect_timeout(CURL_CON_TIME_OUT);
    set_timeout(CURL_TIME_OUT);
    set_data_slot(curl_rsp_t::write_rsp_data_func, &m_rsp);
}

size_t curl_rsp_t::write_rsp_data_func(void *buffer, size_t size, size_t nmemb, void *userp)
{
    LOG << size * nmemb << " bytes written " << userp << ", string: "<< (const char*)buffer << std::endl;
    return size * nmemb;
}

void curl_req_t::on_rsp(long rsp_code, long data_result)
{
    rsp().set_rsp_code(rsp_code);
    rsp().set_curl_code(data_result);
    if (m_my_chroutine != INVALID_ID) {
        ENGIN.awake_chroutine(m_my_chroutine);
    }
}

curl_rsp_t::curl_rsp_t(curl_rsp_t && other) 
{
    uint8_t *tmp = m_buf;
    m_buf = other.m_buf;
    m_rsp_code = other.m_rsp_code;
    m_data_result = other.m_data_result;
    other.m_buf = tmp;
}

}