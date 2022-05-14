// stroke.h
#pragma once

#include <cstdint>
#include <string>
#include <memory>

using namespace stenosys;

namespace stenosys
{

class C_utf8
{

public:

    C_utf8( const std::string & steno );
    ~C_utf8(){}

    bool
    get_first( uint32_t & code );

    bool
    get_next( uint32_t & code );

    int
    length();

private:

    C_utf8(){}

private:

    std::string str_;
    int         index_curr_;

};

}
