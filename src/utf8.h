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

    C_utf8();

    C_utf8( const std::string & steno );
    ~C_utf8(){}

    C_utf8 &
    operator=( const C_utf8 & rhs );

    bool
    get_first( uint32_t & code );

    bool
    get_next( uint32_t & code );

    size_t
    length();

    static int
    differs_at( const std::string & str1, const std::string & str2 );

    void
    append( const std::string & str );

    std::string
    substr( int pos );
 
    const std::string
    str() { return str_; }

    const char  *
    c_str() { return str_p_; }

    static void
    test();

private:

    static size_t 
    length( uint8_t ch );

    static uint32_t
    unpack( const char * data );

    bool
    decode( uint32_t & code );
    
    int 
    to_offset( int pos );

private:

    std::string  str_;
    const char * str_p_;     // pointer to underlying string
    int          index_;     // current index into str_
    int          length_;    // length of str_

};

}
