

#include <X11/Intrinsic.h>
#include <cassert>
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
    : str_( str )
    , index_( 0 )
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

// Calculate string length in UTF-8 codepoints
int
C_utf8::length()
{
    int index  = 0;
    int length = 0;

    while ( index < ( length_ ) )
    {
        int cpl = codepoint_length( str_[ index ] ); 
        length++;
        index += cpl;
    }

    return length;
}

void
C_utf8::test()
{
    // https://en.wikipedia.org/wiki/UTF-8#Examples
    C_utf8 U0024 ( "\x24"             );
    C_utf8 U00A3 ( "\xc2\xa3"         );
    C_utf8 U0939 ( "\xe0\xA4\xb9"     );
    C_utf8 U20AC ( "\xe2\x82\xac"     );
    C_utf8 UD55C ( "\xed\x95\x9c"     );
    C_utf8 U10348( "\xf0\x90\x8d\x88" );

    C_utf8 U10348x2( "\xf0\x90\x8d\x88\xf0\x90\x8d\x88" );
    C_utf8 U10348x3( "\xf0\x90\x8d\x88\xf0\x90\x8d\x88\xf0\x90\x8d\x88" );

    uint32_t code = 0;

    assert( U0024.get_first( code ) );
    assert( code == 0x00000024 );
    assert( U0024.length() == 1 );

    assert( U00A3.get_first( code ) );
    assert( code == 0x000000A3 );
    assert( U00A3.length() == 1 );

    assert( U0939.get_first( code ) );
    assert( code == 0x00000939 );
    assert( U0939.length() == 1 );

    assert( U20AC.get_first( code ) );
    assert( code == 0x000020AC );
    assert( U20AC.length() == 1 );

    assert( UD55C.get_first( code ) );
    assert( code == 0x0000D55C );
    assert( UD55C.length() == 1 );

    assert( U10348.get_first( code ) );
    assert( code == 0x00010348 );
    assert( U10348.length() == 1 );

    assert( U10348x2.length() == 2 );
    assert( U10348x3.length() == 3 );

    fprintf( stdout, "C_utf8::test: PASS\n" );
}

bool
C_utf8::decode( uint32_t & code )
{
    code = '?';

    uint8_t b1 = str_[ index_ ];

    int utf8_length = codepoint_length( b1 );

    switch ( utf8_length )
    {
        case 1:
            if ( index_ < length_ )
            {
                code = b1;
                index_++;
                return true;
            }
            break;
            
        case 2:
            if ( ( index_ + 1 ) < length_ )
            {
                uint8_t b2 = str_[ index_ + 1 ];

                code = ( ( b1 & 0x1f ) << 6 ) + ( b2 & 0x3f );
                index_ += 2;
                return true;
            }
            break;
        
        case 3:
            if ( ( index_ + 2 ) < length_ )
            {
                uint8_t b2 = str_[ index_ + 1 ];
                uint8_t b3 = str_[ index_ + 2 ];

                code = ( ( b1 & 0x0f ) << 12 ) + ( ( b2 & 0x3f ) << 6 ) + ( b3 & 0x3f );
                index_ += 3;
                return true;
            }
            break;

        case 4:
            if ( ( index_ + 3 ) < length_ )
            {
                uint8_t b2 = str_[ index_ + 1 ];
                uint8_t b3 = str_[ index_ + 2 ];
                uint8_t b4 = str_[ index_ + 3 ];

                code = ( ( b1 & 0x07 ) << 18 ) + ( ( b2 & 0x3f ) << 12 )+ ( ( b3 & 0x3f ) << 6 ) + ( b4 & 0x3f );
                index_ += 4;
                return true;
            }
            break;
    }

    return false;
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
