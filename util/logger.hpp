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
#ifdef DEBUG_BUILD
#include "spdlog/sinks/stdout_color_sinks.h"
#endif

namespace chr {

#define MAX_LOG_LEN  512

#define TRACE       ::spdlog::level::trace
#define DEBUG       ::spdlog::level::debug
#define INFO        ::spdlog::level::info
#define WARN        ::spdlog::level::warn
#define ERROR       ::spdlog::level::err
#define CRITICAL    ::spdlog::level::critical
#define OFF         ::spdlog::level::off

class logger_t
{
private:
    logger_t(){
        _async_file = ::spdlog::daily_logger_mt<::spdlog::async_factory_nonblock>("ENGINE", "ENGINE.log", 0, 0);
        
#ifdef DEBUG_BUILD
        _async_file->set_level(TRACE);
        _console = ::spdlog::stdout_color_mt("console");
#else
        _async_file->set_level(INFO);
#endif
    }

public:
    static logger_t &instance() {
        static logger_t instance;
        return instance;
    }
    ~logger_t(){
        ::spdlog::drop_all();
        ::spdlog::shutdown();
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

    void flush() {
        if (_async_file) {
            _async_file->flush();
        }
    }

private:
    std::ostream &m_stream = std::cout;   //consol

public:
    std::shared_ptr<::spdlog::logger> _async_file;
    std::shared_ptr<::spdlog::logger> _console;
};

}

#define LOG chr::logger_t::instance()

#ifdef DEBUG_BUILD
#define SPDLOG(level, format, ...) {\
    if (chr::logger_t::instance()._async_file->should_log(level)) {\
        chr::logger_t::instance()._async_file->log(level, format,  ##__VA_ARGS__);\
        chr::logger_t::instance()._console->log(level, format,  ##__VA_ARGS__);\
    }\
}
#else
#define SPDLOG(level, format, ...) {\
    if (chr::logger_t::instance()._async_file->should_log(level)) {\
        chr::logger_t::instance()._async_file->log(level, format,  ##__VA_ARGS__);\
    }\
}
#endif

#endif