
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

C_formatter::C_formatter( space_type space_mode )
    : space_mode_( space_mode )
{
}

C_formatter::~C_formatter()
{
}

std::string
C_formatter::format( alphabet_type     alphabet_mode
                   , const std::string latin
                   , const std::string shavian
                   , uint16_t          flags_curr
                   , uint16_t          flags_prev 
                   , bool              extends )
{
    bool latin_mode = ( alphabet_mode == AT_LATIN );

    std::string formatted = latin_mode ? latin : shavian;
    
    if ( formatted.length() > 0 )
    {
        if ( latin_mode )
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
        else  // shavian mode
        {
            if ( flags_prev & CAPITALISE_NEXT )
            {
                // Prefix shavian with a naming dot
                formatted = std::string( "·" ) + formatted;
            }
        }
    }
    
    return formatted;
}

std::string
C_formatter::transition_to( const std::string & prev
                          , const std::string & curr
                          , uint16_t            flags_curr
                          , uint16_t            flags_prev 
                          , bool                extends
                          , bool                undo )
{
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "transition_to()" );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  prev   : >%s<", ctrl_to_text( prev ).c_str() );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  curr   : >%s<", ctrl_to_text( curr).c_str() );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  extends: %d", extends );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  undo   : %d", undo );

    std::string output;
    std::string backspaces;
    std::string difference;
   
    bool add_space_before = false;
    bool add_space_after  = false;
    
    if ( extends )
    {
        // Minimise the number of characters required to move from one translation
        // to the next. Check for text common to both.

        //int idx = find_point_of_difference( curr, prev );
        int idx = C_utf8::differs_at( curr, prev );

        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "idx: %d", idx );

        
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

        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "backspaces.length(): %u", backspaces.length() );
        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "difference         : %s", difference.c_str() );
    }
    else // Not extending the previous chord
    {
        if ( undo )
        {
            // Erase the current translation
            C_utf8 utf8_curr( curr );

            backspaces.assign( utf8_curr.length(), '\b' );

            log_writeln( C_log::LL_INFO, LOG_SOURCE, "Undo: not extending" );
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  backspaces.length(): %u", backspaces.length() );
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  attach_to_next     : %d", attach_to_next( flags_prev ) );
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  attach_to_prev     : %d", attach_to_previous( flags_curr ) );

            if ( space_after() )
            {
                if ( ! attach_to_previous( flags_curr, flags_prev ) )
                {
                    // Undo the space-after that would have followed the stroke output
                    backspaces.append( "\b" );
                }
            }
            else // space_before
            {
                //TBW
            }
        }
        else
        {
            if ( space_after() )
            {
                // Send the current translation
                difference = curr;

                add_space_after = ( ! attach_to_previous( flags_curr, flags_prev ) );
            }
            else // space_before
            {
                //TBW        
            }
        }
    }

    output = backspaces;
    
    if ( add_space_before )
    {
        output += " ";
    }

    output += difference;
    
    if ( add_space_after )
    {
        output += " ";
    }

    return output;  
}

int
C_formatter::find_point_of_difference( const std::string & from, const std::string & to )
{
    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "find pod  --  from: >>%s<<  to: >>%s<<"
                                               //, from.c_str()
                                               //, to.c_str() );

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


// Attach to the previous stroke's output?
bool
C_formatter::attach_to_previous( uint16_t flags_curr, uint16_t flags_prev )
{
    return ( ( flags_prev   & ATTACH_TO_NEXT )     ||
             ( flags_curr   & ATTACH_TO_PREVIOUS ) ||
             ( ( flags_prev & GLUE ) && ( flags_curr & GLUE ) ) );
}

bool
C_formatter::attach_to_next( uint16_t flags )
{
    return flags & ATTACH_TO_NEXT;
}

bool
C_formatter::attach_to_previous( uint16_t flags )
{
    return flags & ATTACH_TO_PREVIOUS;
}

bool
C_formatter::space_after()
{
    return space_mode_ == SP_AFTER;
}

bool
C_formatter::space_before()
{
    return space_mode_ == SP_BEFORE;
}

}
