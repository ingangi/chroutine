/// \file chan_selecter.hpp
/// 
/// chan_selecter_t is for multi-channel processing in chroutine
///
/// \author ingangi
/// \version 0.1.0
/// \date 2019-04-02

#ifndef CHAN_SELECTER_H
#define CHAN_SELECTER_H

#include "channel.hpp"

namespace chr {

// not thread safe, use it in the same chroutine.
class chan_selecter_t final
{
    typedef enum {
        selecter_type_unknown = 0,
        selecter_type_default,
        selecter_type_read,
        // selecter_type_write,
    } act_type_t;

    typedef std::function<void()> callback_t;
    typedef struct {
        channel_it*         chan = nullptr;
        act_type_t          type = selecter_type_unknown;
        void *              data_ptr = nullptr;
        callback_t          callback = nullptr;
    } chan_select_node_t;

    typedef std::map<void *, chan_select_node_t> cases_t;

public:
    chan_selecter_t();
    ~chan_selecter_t();

    int add_case(channel_it* chan, /*act_type_t type, */void *data_ptr, const callback_t& callback);
    int del_case(channel_it* chan);
    void default_case(const callback_t& callback);
    void select();

// private:
//     void shuffle_cases();
    
private:
    chan_selecter_t(const chan_selecter_t&) = delete;
    chan_selecter_t(chan_selecter_t&&) = delete;
    chan_selecter_t& operator=(const chan_selecter_t&) = delete;
    chan_selecter_t& operator=(chan_selecter_t&&) = delete;
    cases_t::iterator select_once();

private:
    chan_select_node_t m_default;
    cases_t         m_cases;
};

}

#endif