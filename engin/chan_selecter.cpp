#include "chan_selecter.hpp"

namespace chr {

chan_selecter_t::chan_selecter_t()
{}

chan_selecter_t::~chan_selecter_t()
{}

int chan_selecter_t::add_case(channel_it* chan, /*act_type_t type, */void *data_ptr, const callback_t& callback)
{
    if (chan == nullptr || data_ptr == nullptr) {
        SPDLOG(ERROR, "{} error: chan(0x{0:x}), data_ptr(0x{0:x})", __FUNCTION__, chan, data_ptr);
        return -1;
    }

    auto iter_find = m_cases.find(chan);
    if (iter_find != m_cases.end()) {
        SPDLOG(ERROR, "{} error:key conflict(will delete the old one): 0x{0:x}", __FUNCTION__, chan);
        m_cases.erase(iter_find);
    }

    chan_select_node_t tmp;
    tmp.chan = chan;
    tmp.type = selecter_type_read; //type;
    tmp.callback = std::move(callback);
    tmp.data_ptr = data_ptr;
    m_cases[chan] = tmp;
    return 0;
}

void chan_selecter_t::default_case(const callback_t& callback)
{
    m_default.type = selecter_type_default;
    m_default.callback = std::move(callback);
}

int chan_selecter_t::del_case(channel_it* chan)
{
    auto iter_find = m_cases.find(chan);
    if (iter_find != m_cases.end()) {
        SPDLOG(DEBUG, "{} delete key: 0x{0:x}", __FUNCTION__, chan);
        m_cases.erase(iter_find);
    }

    return 0;
}

#define SYNC_CASE
int chan_selecter_t::select_once() 
{
    int called = 0;
    cases_t::iterator iter = m_cases.begin();
    for (; iter != m_cases.end(); iter++) {
        auto &node = iter->second;
        if (node.chan == nullptr) {
            continue;
        }

        if (node.type == selecter_type_read) {
            if (node.chan->read(node.data_ptr, true)) {
                called++;
#ifdef SYNC_CASE
                node.callback();
#else
                // callbacks run in other chroutine
                ENGIN.create_son_chroutine([&](void *){
                    node.callback();
                }, nullptr);
#endif
                break;
            }
        } else {
            SPDLOG(ERROR, "{} error: unexpect type: {}", __FUNCTION__, node.type);
        }
    }

    return called;
}

void chan_selecter_t::select()
{
    for (;;) {
        if (select_once() > 0) {
            // shuffle_cases();
            break;
        }

        if (m_default.callback != nullptr) {
            // call the default case
#ifdef SYNC_CASE
            m_default.callback();
#else
            ENGIN.create_son_chroutine([&](void *){
                m_default.callback();
            }, nullptr);
#endif
            break;
        }

        // no default case set
        SLEEP(10);
    }
}


}