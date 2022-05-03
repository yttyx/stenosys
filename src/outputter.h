#pragma once

#include <string>

namespace stenosys
{

enum formatter_mode { FM_NONE, FM_CONSOLE, FM_ARDUINO };

class C_outputter
{

public:

    C_outputter( formatter_mode mode ) { mode_ = mode; }
    ~C_outputter(){}

    std::string
    format( const std::string & text );

private:

    C_outputter(){}

private:

    formatter_mode mode_;
};

}
