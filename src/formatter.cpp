
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "formatter.h"
#include "log.h"
#include "miscellaneous.h"
#include "stenoflags.h"
#include "utf8.h"

#define LOG_SOURCE "FRMTR"

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_formatter::C_formatter()
{
}

C_formatter::~C_formatter()
{
}

std::string
C_formatter::format( alphabet_type     alphabet_mode
                   , const std::string text
                   , uint16_t          flags_curr
                   , uint16_t          flags_prev 
                   , bool              extends )
{
    std::string space_before;
    std::string space_after;

    std::string formatted = text;
    
    if ( formatted.length() > 0 )
    {
        if ( ( space_mode_ == SP_BEFORE ) && ( ! attach( flags_prev, flags_curr ) ) )
        {
            // Insert a space
            space_before = " ";
        }

        if ( alphabet_mode == AT_LATIN )
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
        }
        else  // Shavian alphabet
        {
            if ( flags_prev & NAMING_DOT )
            {
                // Prefix shavian with a naming dot
                formatted = std::string( "·" ) + formatted;
            }
        }
    
        if ( space_mode_ == SP_AFTER )
        {
            // Always insert a space. If the following stroke turns out to be attached
            // to this one, the space will need to be removed (backspaced over).
            space_after = " ";
        }
    }
    
    return space_before + formatted + space_after;
}

std::string
C_formatter::transition_to( const std::string & prev
                          , const std::string & curr
                          , uint16_t            flags_curr
                          , uint16_t            flags_prev 
                          , bool                extends
                          , bool                undo )
{
    //log_writeln( C_log::LL_INFO, LOG_SOURCE, "transition_to()" );
    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  prev   : >%s<", ctrl_to_text( prev ).c_str() );
    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  curr   : >%s<", ctrl_to_text( curr).c_str() );
    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  extends: %d", extends );
    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  undo   : %d", undo );

    std::string output;
    std::string backspaces;
    std::string difference;
   
    if ( extends )
    {
        // Minimise the number of characters required to move from one translation
        // to the next. Check for text common to both.

        int idx = C_utf8::differs_at( curr, prev );
        
        if ( idx >= 0 )
        {
            if ( undo )
            {
                C_utf8 utf8_curr( curr );
                C_utf8 utf8_prev( prev );

                backspaces.assign( utf8_curr.length() - idx, '\b' );

                difference = utf8_prev.substr( idx );
            }
            else
            {
                C_utf8 utf8_curr( curr );
                C_utf8 utf8_prev( prev );
                
                backspaces.assign( utf8_prev.length() - idx, '\b' );
                
                difference = utf8_curr.substr( idx );
            }
        }
        else {

            if ( undo )
            {
                C_utf8 utf8_curr( curr );

                backspaces.assign( utf8_curr.length(), '\b' );
                difference = prev;
            }
            else
            {
                C_utf8 utf8_prev( prev );
                
                backspaces.assign( utf8_prev.length(), '\b' );
                difference = curr;
            }
        }
    }
    else // Not extending the previous chord
    {
        if ( undo )
        {
            // Erase the current translation
            C_utf8 utf8_curr( curr );

            backspaces.assign( utf8_curr.length(), '\b' );
        }
        else
        {
            // Send the current translation
            difference = curr;
        }
    }

    return backspaces + difference;  
}

int
C_formatter::find_point_of_difference( const std::string & from, const std::string & to )
{
    uint16_t ii = 0;
    
    for ( ; ( ii < from.length() ) && ( ii < to.length() ); ii++ )
    {
        if ( from[ ii ] != to[ ii ] )
        {
            break;
        }
    }

    if ( ( ii < from.length() ) && ( ii < to.length() ) )
    {
        return ( int ) ii;
    }

    return -1;
}


void
C_formatter::space_mode( space_type space_mode )
{
    space_mode_ = space_mode;
}

// Attach to the previous stroke's output?
bool
C_formatter::attach( uint16_t flags_prev, uint16_t flags_curr )
{
    return ( ( flags_prev   & ATTACH_TO_NEXT )     ||
             ( flags_curr   & ATTACH_TO_PREVIOUS ) ||
             ( ( flags_prev & GLUE ) && ( flags_curr & GLUE ) ) );
}

}
