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
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/daily_file_sink.h"

#define MAX_LOG_LEN  512

#define TRACE       ::spdlog::level::trace
#define DEBUG       ::spdlog::level::debug
#define INFO        ::spdlog::level::info
#define WARN        ::spdlog::level::warn
#define ERROR       ::spdlog::level::err
#define CRITICAL    ::spdlog::level::critical
#define OFF         ::spdlog::level::off


namespace chr {


class logger_t
{
private:
    logger_t(){
        _async_file = ::spdlog::daily_logger_mt<::spdlog::async_factory_nonblock>("ENGINE", "ENGINE.log", 0, 0);
    }

public:
    static logger_t &instance() {
        static logger_t instance;
        return instance;
    }
    ~logger_t(){
        ::spdlog::shutdown();
        ::spdlog::drop_all();
    }

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
    std::ostream &m_stream = std::cout;   //consol

public:
    std::shared_ptr<::spdlog::logger> _async_file;
};

}

#define LOG chr::logger_t::instance()
#define SPDLOG(level, format, ...) {chr::logger_t::instance()._async_file->log(level, format,  ##__VA_ARGS__);}

#endif