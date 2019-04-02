#include "chan_selecter.hpp"

namespace chr {

chan_selecter_t::chan_selecter_t()
{}

chan_selecter_t::~chan_selecter_t()
{}

int chan_selecter_t::add_case(channel_it* chan, /*act_type_t type, */void *data_ptr, const callback_t& callback)
{
    if (chan == nullptr || data_ptr == nullptr) {
        LOG("%s error: chan(%p), data_ptr(%p)", __FUNCTION__, chan, data_ptr);
        return -1;
    }

    auto iter_find = m_cases.find(chan);
    if (iter_find != m_cases.end()) {
        LOG << __FUNCTION__ << " error:key conflict(will delete the old one):" << chan << std::endl;
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
        LOG << __FUNCTION__ << " delete key:" << chan << std::endl;
        m_cases.erase(iter_find);
    }

    return 0;
}

chan_selecter_t::cases_t::iterator chan_selecter_t::select_once() 
{
    cases_t::iterator iter = m_cases.begin();
    for (; iter != m_cases.end(); iter++) {
        auto &node = iter->second;
        if (node.chan == nullptr) {
            continue;
        }

        if (node.type == selecter_type_read) {
            if (node.chan->read(node.data_ptr, true)) {
                node.callback();
                break;
            }
        } else {
            LOG << __FUNCTION__ << " error: unexpect type:" << node.type << std::endl;
        }
    }

    return iter;
}

void chan_selecter_t::select()
{
    for (;;) {
        auto iter = select_once();
        if (iter == m_cases.end()) {
            // call the default case
            if (m_default.callback != nullptr) {
                m_default.callback();
                break;
            } else {
                SLEEP(10);
            }
        } else {
            break;
        }
    }
}


}