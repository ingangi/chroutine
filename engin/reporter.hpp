/// \file reporter.hpp
/// 
/// reporter_t is for the communication between father and son croutines
///
/// \author ingangi
/// \version 0.1.0
/// \date 2018-12-24

#ifndef REPORTER_H
#define REPORTER_H

#include <memory>

typedef enum {
    result_ready = 0,  // not excuted
    result_done,  // excuted
    result_error,
    result_timeout,
} son_result_t;

class reporter_base_t
{
public:
    virtual void set_result(son_result_t result) = 0;
    virtual void set_data(void * pdata) = 0;
    virtual void *get_data() = 0;
    virtual son_result_t get_result() = 0;
};

typedef std::shared_ptr<reporter_base_t> reporter_sptr_t;

// reporter_t used between father and son chroutin,
// and they run in the same os thread
template<typename T>
class reporter_t : public reporter_base_t
{
public:
    virtual ~reporter_t(){}
    static reporter_sptr_t create() {
        return reporter_sptr_t(new reporter_t<T>);
    }
    
    virtual void set_result(son_result_t result) {
        errcode = result;
    }
    virtual void set_data(void * pdata) {
        data = *(static_cast<T*>(pdata));
    }
    virtual void *get_data() {
        return &data;
    }
    virtual son_result_t get_result() {
        return errcode;
    }

private:
    reporter_t() : errcode(result_ready) {}

private:
    T           data;   // &data is the 'void *arg' for son's func
    son_result_t    errcode;

};


#endif