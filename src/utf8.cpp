

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
    , str_p_( str.c_str() )
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

int
C_utf8::length()
{
    return C_utf8::length( str_ );
}

void
C_utf8::test()
{
    // https://en.wikipedia.org/wiki/UTF-8#Examples
    uint32_t code = 0;

    C_utf8 U0024 ( "\x24" );
    assert( U0024.get_first( code ) );


    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "C_utf8::test(): code %u", code );


    assert( code == 0x00000024 );
    assert( U0024.length() == 1 );

    C_utf8 U00A3 ( "\xc2\xa3"         );
    assert( U00A3.get_first( code ) );
    assert( code == 0x000000A3 );
    assert( U00A3.length() == 1 );

    C_utf8 U0939 ( "\xe0\xA4\xb9"     );
    assert( U0939.get_first( code ) );
    assert( code == 0x00000939 );
    assert( U0939.length() == 1 );

    C_utf8 U20AC ( "\xe2\x82\xac"     );
    assert( U20AC.get_first( code ) );
    assert( code == 0x000020AC );
    assert( U20AC.length() == 1 );

    C_utf8 UD55C ( "\xed\x95\x9c"     );
    assert( UD55C.get_first( code ) );
    assert( code == 0x0000D55C );
    assert( UD55C.length() == 1 );

    C_utf8 U10348( "\xf0\x90\x8d\x88" );
    assert( U10348.get_first( code ) );
    assert( code == 0x00010348 );
    assert( U10348.length() == 1 );

    C_utf8 U10348x2( "\xf0\x90\x8d\x88\xf0\x90\x8d\x88" );
    assert( U10348x2.length() == 2 );
    
    C_utf8 U10348x3( "\xf0\x90\x8d\x88\xf0\x90\x8d\x88\xf0\x90\x8d\x88" );
    assert( U10348x3.length() == 3 );

    fprintf( stdout, "C_utf8::test: PASS\n" );
}

bool
C_utf8::decode( uint32_t & code )
{
    code = '?';

    if ( index_ < length_ )
    {
        uint8_t b1 = *( str_p_ + index_ );


        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, " str_p       : %xh,%xh,%xh,%xh"
                                                   , *( str_p_ + 0 )
                                                   , *( str_p_ + 1 )
                                                   , *( str_p_ + 2 )
                                                   , *( str_p_ + 3 ) );

        int utf8_length = length( b1 );


        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  b1         : %xh", b1 );
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  utf8_length: %d",  utf8_length );
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  index_     : %d",  index_ );

        code = unpack( str_p_ + index_);

        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  code       : %xh",   code );

        index_ += utf8_length;
        
        return true;
    }

    return false;
}

// Calculate string length in UTF-8 codepoints
int
C_utf8::length( const std::string & str )
{
    int index   = 0;
    int strlen  = str.length();
    int utf8len = 0;

    const char * p = str.c_str();;

    while ( index < strlen )
    {
        index += length( *( p + index ) );
        utf8len++;
    }

    return utf8len;
}

// Calculate length of single codepoint in bytes
int
C_utf8::length( uint8_t ch )
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

// Extract one UTF-8 character from a buffer
uint32_t
C_utf8::unpack( const char * data )
{
    uint32_t code = '?';

    uint8_t b1 = *data;

    int len = length( b1 );
    
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  Unpack, b1 : %02xh", b1 );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  Unpack, len: %d", len );

    switch ( len )
    {
        case 1:
        {
            code = b1;
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  Unpack, code: %02xh", code );
            break;
        }    
        case 2:
        {
            uint8_t b2 = *( data + 1 );
            code = ( ( b1 & 0x1f ) << 6 ) + ( b2 & 0x3f );
            break;
        }
        case 3:
        {
            uint8_t b2 = *( data + 1 );
            uint8_t b3 = *( data + 2 );
            code = ( ( b1 & 0x0f ) << 12 ) + ( ( b2 & 0x3f ) << 6 ) + ( b3 & 0x3f );
            break;
        }
        case 4:
        {
            uint8_t b2 = *( data + 1 );
            uint8_t b3 = *( data + 2 );
            uint8_t b4 = *( data + 3 );
            code = ( ( b1 & 0x07 ) << 18 ) + ( ( b2 & 0x3f ) << 12 )+ ( ( b3 & 0x3f ) << 6 ) + ( b4 & 0x3f );
            break;
        }
    }

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  Unpack, code: %02xh", code );
    
    return code;
}

}
