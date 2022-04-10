#pragma once

#include <string>

namespace stenosys
{

enum formatter_mode { FM_NONE, FM_CONSOLE, FM_ARDUINO };

class C_formatter
{

public:

    C_formatter( formatter_mode mode ) { mode_ = mode; }
    ~C_formatter(){}

    std::string
    format( const std::string & text );

private:

    C_formatter(){}

private:

    formatter_mode mode_;
};

}