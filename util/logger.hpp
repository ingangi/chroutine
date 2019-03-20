/// \file logger.hpp
/// 
/// log adapter
///
/// \author ingangi
/// \version 0.1.0
/// \date 2019-03-20

#include <iostream>

class logger_t
{
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

    logger_t& operator << ( std::ostream& (*op) (std::ostream&)) {
        (*op) (m_stream);
		return (*this);
    }

private:
    std::ostream &m_stream = std::cout;   //replace with your log lib
};

#define LOG logger_t::instance()