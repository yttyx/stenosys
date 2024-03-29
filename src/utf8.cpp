#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "log.h"
#include "miscellaneous.h"
#include "utf8.h"


using namespace stenosys;

namespace stenosys
{

extern C_log log;


C_utf8::C_utf8()
    : str_( "" )
    , str_p_( str_.c_str() )
    , index_( 0 )
    , length_( 0 )
{
}

C_utf8::C_utf8( const std::string & str )
    : str_( str )
    , str_p_( str_.c_str() )
    , index_( 0 )
    , length_( str.length() )
{
}

// copy assignment
C_utf8 &
C_utf8::operator=( const C_utf8 & rhs )
{
    // Guard self assignment
    if ( this == &rhs)
    {
        return *this;
    }

    str_    = rhs.str_;
    str_p_  = str_.c_str();
    index_  = 0;
    length_ = str_.length();

    return *this;
}

bool
C_utf8::get_first( uint32_t & code )
{
    index_ = 0;
    return decode( code, true );
}

bool
C_utf8::get_next( uint32_t & code )
{
    return decode( code, true );
}

bool
C_utf8::peek_next( uint32_t & code )
{
    return decode( code, false );
}

bool
C_utf8::get_first( std::string  & str )
{
    index_ = 0;
    return decode( str, true );
}

bool
C_utf8::get_next( std::string  & str )
{
    return decode( str, true );
}

bool
C_utf8::peek_next( std::string & str )
{
    return decode( str, false );
}

void
C_utf8::consume_next()
{
    if ( index_ < length_ )
    {
        uint8_t b1 = *( str_p_ + index_ );

        int utf8_length = length( b1 );

        index_ += utf8_length;
    }
}

bool
C_utf8::find( const std::string & s )
{
    return str_.find( s ) != std::string::npos;
}

bool
C_utf8::find( char ch )
{
    return str_.find( ch ) != std::string::npos;
}

size_t
C_utf8::length()
{
    int offset  = 0;
    int utf8len = 0;

    while ( offset < length_ )
    {
        offset += length( *( str_p_ + offset) );
        utf8len++;
    }

    return utf8len;
}

int
C_utf8::differs_at( const std::string & str1, const std::string & str2 )
{
    C_utf8 s1( str1 );
    C_utf8 s2( str2 );

    uint32_t code1 = 0;
    uint32_t code2 = 0;
    size_t   count = 0;

    if ( s1.get_first( code1 ) && s2.get_next( code2 ) )
    {
        do
        {
            if ( code1 != code2 )
            {
                break;
            }

            count++;

        } while ( s1.get_next( code1 ) && s2.get_next( code2 ) );
    }

    return ( ( count < s1.length() ) && ( count < s2.length() ) ) ? count : -1;     
}

// Returns the *remainder* of the string starting at UTF-8 character position pos
std::string
C_utf8::at( int pos )
{
    int utf8_len = length();

    if ( ( utf8_len == 0 ) || ( pos > utf8_len ) )
    {
        return std::string( "" );
    }

    int offset = to_offset( pos );

    size_t len = length( str_[ offset ] );

    return str_.substr( offset, len );
}

// Returns the single UTF-8 character at position pos
std::string
C_utf8::substr( int pos )
{
    int utf8_len = length();

    if ( ( utf8_len == 0 ) || ( pos > utf8_len ) )
    {
        return std::string( "" );
    }

    int offset = to_offset( pos );

    return str_.substr( offset );
}


void
C_utf8::append( const std::string & str )
{
    str_ += str;

    length_ = str_.length();
}

bool
C_utf8::decode( uint32_t & code, bool update_index )
{
    code = '?';

    if ( index_ < length_ )
    {
        uint8_t b1 = *( str_p_ + index_ );

        int utf8_length = length( b1 );
        
        code = unpack( str_p_ + index_);

        if ( update_index )
        {
            index_ += utf8_length;
        }

        return true;
    }

    return false;
}

bool
C_utf8::decode( std::string & str, bool update_index )
{
    if ( index_ < length_ )
    {
        uint8_t b1 = *( str_p_ + index_ );

        int utf8_length = length( b1 );

        str = str_.substr( index_, utf8_length );

        if ( update_index )
        {
            index_ += utf8_length;
        }

        return true;
    }

    return false;
}

// Convert UTF8 character index in string to an absolute string offset
int 
C_utf8::to_offset( int pos )
{
    int offset  = 0;

    while ( pos-- )
    {
        offset += length( *( str_p_ + offset ) );
    }

    return offset;
}

// Calculate length of single codepoint in bytes
size_t
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
    
    switch ( len )
    {
        case 1:
        {
            code = b1;
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

    return code;
}


void
C_utf8::dump()
{
    log_writeln( C_log::LL_INFO, "C_utf8::dump()" );

    //TODO byte dump of string (ctrl_to_text was modified to make it friendlier
    //     for doing the strokes dump (see C_strokes) but this isn't what we want
    //     here.

    std::string formatted;

    ctrl_to_text( str_, formatted );

    log_writeln_fmt( C_log::LL_INFO, "  str_    : %s", formatted.c_str() );
    log_writeln_fmt( C_log::LL_INFO, "  index_  : %d", index_ );
    log_writeln_fmt( C_log::LL_INFO, "  length_ : %d", length_ );
    
    log_writeln_fmt( C_log::LL_INFO, "sizeof( char): %d", sizeof( char ) );
}

void
C_utf8::test()
{
    // https://en.wikipedia.org/wiki/UTF-8#Examples
    uint32_t code = 0;

    C_utf8 U0024 ( "\x24" );
    assert( U0024.get_first( code ) );

    log_writeln_fmt( C_log::LL_INFO, "C_utf8::test(): code %u", code );

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

}
