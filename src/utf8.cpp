

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "common.h"
#include "log.h"
#include "utf8.h"

#define LOG_SOURCE "STRK "

using namespace stenosys;

namespace stenosys
{

extern C_log log;


C_utf8::C_utf8( const std::string & str )
    : index_( 0 )
    , length_( str.length() )
{
}


bool
C_utf8::get_first( uint32_t & code )
{
    index_ = 0;
    return decode( code );
}

bool
C_utf8::get_next( uint32_t & code )
{
    return decode( code );
}

bool
C_utf8::decode( uint32_t & code )
{
    uint32_t result = '?';

    uint8_t b1 = str_[ index_ ];

    int utf8_length = codepoint_length( b1 );

    switch ( utf8_length )
    {
        case 1:
            result = b1;
            index_++;
            break;

        case 2:
            if ( index_ + 1 < length_ )
            {
                uint8_t b2 = str_[ index_ + 1 ];

                result = ( ( b1 & 0x1f ) << 6 ) + ( b2 & 0x3f );
            }
            break;
        
        case 3:
            if ( index_ + 2 < length_ )
            {
                uint8_t b2 = str_[ index_ + 1 ];
                uint8_t b3 = str_[ index_ + 2 ];

                result = ( ( b1 & 0x0f ) << 12 ) + ( ( b2 & 0x3f ) << 6 ) + ( b3 & 0x3f );
            }
            break;

        case 4:
            if ( index_ + 3 < length_ )
            {
                uint8_t b2 = str_[ index_ + 1 ];
                uint8_t b3 = str_[ index_ + 2 ];
                uint8_t b4 = str_[ index_ + 3 ];

                result = ( ( b1 & 0x07 ) << 18 ) + ( ( b2 & 0x3f ) << 12 )+ ( ( b3 & 0x3f ) << 6 ) + ( b4 & 0x3f );
            }
            break;
    }

    return result;
}

int
C_utf8::codepoint_length( uint8_t ch )
{
    if ( ( ch & 0x80 ) == 0 )
    {
        return 1;
    }

    if ( ( ch & 0xf8 ) == 0xf0 )
    {
        return 4;
    }

    if ( ( ch & 0xe0 ) == 0xc0 )
    {
        return 2;
    }

    if ( ( ch & 0xf0 ) == 0xe0 )
    {
        return 3;
    }

    return 0;
}



}
