
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

//#include <common.h>
#include "formatter.h"
#include "log.h"
#include "stenoflags.h"

#define LOG_SOURCE "FRMTR"

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_formatter::C_formatter( space_type space_mode )
    : space_mode_( space_mode )
{
}

C_formatter::~C_formatter()
{
}

std::string
C_formatter::format( const std::string text
                    , uint16_t         flags
                    , uint16_t         flags_prev 
                    , bool             extends )
{
    /* log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "backspaces: %u", ( uint16_t ) backspaces ); */

    bool config_space_before = ( space_mode_ == SP_BEFORE );
    bool config_space_after  = ! config_space_before;

    // Attach to the previous stroke's output?
    bool attach_to_previous = ( ( flags_prev   & ATTACH_TO_NEXT )     ||
                                ( flags        & ATTACH_TO_PREVIOUS ) ||
                                ( ( flags_prev & GLUE ) && ( flags & GLUE ) ) );

    // Attach to the next stroke's output?
    bool attach_to_next = ( ( flags & ATTACH_TO_NEXT ) || ( flags & GLUE ) ); 

    std::string formatted = text;
    
    std::string output;
    
    if ( formatted.length() > 0 )
    {
        if ( flags_prev & CAPITALISE_NEXT )
        {
            formatted[ 0 ] = toupper( formatted[ 0 ] );
        }
        else if ( flags_prev & LOWERCASE_NEXT )
        {
            formatted[ 0 ] = tolower( formatted[ 0 ] );
        }
        else if ( flags_prev & LOWERCASE_NEXT_WORD )
        {
            std::transform( formatted.begin(), formatted.end(), formatted.begin(), ::tolower );
        }
        else if ( flags_prev & UPPERCASE_NEXT_WORD )
        {
            std::transform( formatted.begin(), formatted.end(), formatted.begin(), ::toupper );
        }

        if ( config_space_before && ( ! attach_to_previous ) )
        {
            output += ' '; 

            log_writeln( C_log::LL_INFO, LOG_SOURCE, "Added leading space" );
        }

        output += formatted;

        if ( config_space_after && ( ! attach_to_next ) )
        {
            output += ' ';
        }
    }
    
    return output;
}

std::string
C_formatter::extend( const std::string & prev, const std::string & curr )
{
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "extend --  prev: %s curr: %s"
                                               , prev.c_str()
                                               , curr.c_str() );

    // Minimise the number of characters required to move from the previous
    // translation to the current translation. Check for text common to both
    // the current and previous translations. Add backspaces to bring the
    // previous translation back to the point of divergence, then add the
    // part of the current translation after that point.
    
    std::string output;
    std::string backspaces;
    std::string extra;

    int idx = find_point_of_difference( prev, curr );

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "idx: %u", idx );
    
    if ( idx >= 0 )
    {
        backspaces.assign( prev.length() - idx, '\b' );

        extra = curr.substr( idx );
    }
    else {
    
        backspaces.assign( prev.length(), '\b' );
        extra = curr;
    }

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "backspaces.length(): %u", backspaces.length() );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "extra              : %s", extra.c_str() );

    output = backspaces;
    output += extra;

    return output;  
}

uint16_t
C_formatter::find_point_of_difference( const std::string & prev, const std::string & curr )
{
    if ( ( curr.length() < prev.length() ) || ( prev == curr ) )
    {
        return -1;
    }

    uint16_t ii = 0;
    
    for ( ; ( ii < prev.length() ) && ( ii < curr.length() ); ii++ )
    {
        if ( prev[ ii ] != curr[ ii ] )
        {
            break;
        }
    }

    if ( ( ii < prev.length() ) && ( ii < curr.length() ) )
    {
        return ii;
    }

    return -1;
}



}
