// utf8.h
#pragma once

#include <cstdint>
#include <string>
#include <memory>

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

    static int
    length( const std::string & str );

    static void
    test();

private:

    C_utf8(){}

    static int
    length( uint8_t ch );

    static uint32_t
    unpack( const char * data );

    bool
    decode( uint32_t & code );
    
private:

    std::string  str_;
    const char * str_p_;     // pointer to underlying string
    int          index_;     // current index into str_
    int          length_;    // length of str_

};

}
