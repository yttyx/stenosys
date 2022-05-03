
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
                    , uint16_t          flags
                    , uint16_t          flags_prev 
                    , bool              extends )
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

        // Only output a leading space if the stroke generated some text
        if ( config_space_before && ( ! attach_to_previous ) && ( formatted.length() > 0 ) )
        {
            output += ' '; 
            //current_stroke->st = SP_BEFORE; 

            log_writeln( C_log::LL_INFO, LOG_SOURCE, "Added leading space" );
        }

        output += formatted;

        // Only output a trailing space if the stroke generated some text
        if ( config_space_after && ( ! attach_to_next ) && ( formatted.length() > 0 ) )
        {
            output += ' ';
        }
    }
    
    return output;
}

}
