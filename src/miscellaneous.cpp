// miscellaneous.cpp

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include "miscellaneous.h"
#include "mutex.h"

namespace stenosys
{

long
file_length( const std::string & filename )
{
    struct stat stat_buf;

    int rc = stat( filename.c_str(), &stat_buf );

    return rc == 0 ? stat_buf.st_size : -1;
}

/** \brief Get current time (HH:MM)

    @param[out]     tm_struct: Time structure populated
*/
void
current_time( tm & tm_struct )
{
    time_t  now = time( 0 );

    tm_struct = *localtime( &now );
}

/** \brief Format time

    @param          tm_struct: Time structure
    @return         Time as string, format HH MM
*/
std::string
format_time_hh_mm( const tm & tm_struct )
{
    char       buf[ 80 ];

    strftime( buf, sizeof( buf ), "%H:%M", &tm_struct );

    return buf;
}

std::string
format_datetime()
{
    time_t    now = time( 0 );
    struct tm tms = *localtime( &now );

    std::string datetime = format_string( "%02d/%02d/%04d %02d:%02d:%02d ", tms.tm_mday, tms.tm_mon + 1, tms.tm_year + 1900,
                                                                            tms.tm_hour, tms.tm_min,     tms.tm_sec );
    return datetime;
}

std::string
format_string( const char * format, ... )
{
    assert( format );

    va_list arg_ptr;
    
    va_start( arg_ptr, format );

    char buf[ 4096 + 1 ];

    vsnprintf( buf, sizeof( buf ) - 1, format, arg_ptr );
    va_end( arg_ptr );

    return std::string( buf );
}

void
ltrim_string( std::string & s )
{
    s.erase( 0, s.find_first_not_of( " \n\r\t" ) );
}

void
rtrim_string( std::string & s )
{
    s.erase( s.find_last_not_of( " \n\r\t" ) + 1 );
}

void
find_and_replace( std::string & source, std::string const & find, std::string const & replace )
{
    for( std::string::size_type ii = 0; ( ii = source.find( find, ii ) ) != std::string::npos; )
    {
        source.replace( ii, find.length(), replace );
        ii += replace.length();
    }
}

// Convert control characters in a string to text
// For example, "\n" becomes [0a]"
std::string
ctrl_to_text( const std::string & text )
{
    std::string output;

    for ( size_t ii = 0; ii < text.length(); ii++ )
    {
        if ( iscntrl( text[ ii ] ) )
        {
            char buffer[ 10 ];

            snprintf( buffer, sizeof( buffer ), "[%02x]", text[ ii ] );
            output += buffer;
        }
        else
        {
            output += text[ ii ];
        }
    }

    return output;
}

void
delay( unsigned int milliseconds)
{
    usleep( 1000 * milliseconds );
}

}
