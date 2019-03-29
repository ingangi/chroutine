/// \file logger.hpp
/// 
/// log adapter
///
/// \author ingangi
/// \version 0.1.0
/// \date 2019-03-20

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <stdarg.h>

namespace chr {

class logger_t
{
    const int MAX_LOG_LEN = 512;
private:
    logger_t(){}

public:
    static logger_t &instance() {
        static logger_t instance;
        return instance;
    }
    ~logger_t(){}

    template <typename T> logger_t& operator<<(const T& value) {
        m_stream << value;
		return (*this);
	}

    logger_t& operator << (std::ostream& (*op) (std::ostream&)) {
        (*op) (m_stream);
		return (*this);
    }

    logger_t& operator () (const char* format, ...) {
        char tmp[MAX_LOG_LEN];
        va_list args;
        va_start(args, format);
        vsnprintf(tmp, MAX_LOG_LEN, format, args);
        va_end(args);
        m_stream << tmp;
        std::endl(m_stream);
        return (*this);
    }

private:
    std::ostream &m_stream = std::cout;   //replace with your log lib
};

}

#define LOG chr::logger_t::instance()
#endif