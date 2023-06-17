// log.cpp
//

#include <assert.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "log.h"
#include "miscellaneous.h"

using namespace  stenosys;

namespace stenosys
{

C_log log;


C_log::C_log()
{
    level_    = LL_INFO;
    datetime_ = false;
    fileline_ = false;
}

C_log::~C_log()
{
}

void
C_log::initialise( eLogLevel level, bool datetime )
{
    level_    = level;
    datetime_ = datetime;

    if ( level_ >= LL_VERBOSE_1 )
    {
        fileline_ = true;
    }
}

void
C_log::write_line( eLogLevel    level
                 , bool         prefix_text
                 , bool         newline
                 , const char * file
                 , int          line
                 , const char * format
                 , ... )
{
    assert( format );

    if ( level <= level_ )
    {
        log_lock_.lock();

        std::string str;
   
        if ( prefix_text )
        {
            if ( fileline_ )
            {
                str = format_string( "%-20.20s line %04d: ", file, line );
            }
        }

        va_list arg_ptr;
        va_start( arg_ptr, format );

        char buf[ 4096 + 1 ];

        vsnprintf( buf, sizeof( buf ) - 1, format, arg_ptr );
        va_end( arg_ptr );

        str += std::string( buf );

        if ( newline )
        {
            str += "\n";
        }

        fprintf( stdout, "%s", str.c_str() );
        fflush( stdout );

        log_lock_.unlock();
    }
}

}
