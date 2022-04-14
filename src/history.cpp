
#include <algorithm>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "common.h"
#include "dictionary.h"
#include "history.h"
#include "log.h"
#include "stenoflags.h"

#define LOG_SOURCE "HIST "

using namespace stenosys;

namespace stenosys
{

extern C_log log;

// Convert control characters in a string to text
C_stroke_history::C_stroke_history()
{
    // Turn the stroke array into a double-linked circular buffer
    for ( std::size_t ii = 0; ii < NUMBEROF( strokes_ ); ii++ )
    {
        C_stroke * stroke = &strokes_[ ii ];

        if ( ii == 0 )
        {
            C_stroke::set_curr( stroke );
        }
    
        stroke->set_prev( &strokes_[ ( ii == 0 ) ? NUMBEROF( strokes_ ) - 1 : ii - 1 ] );
        stroke->set_next( &strokes_[ ( ii == NUMBEROF( strokes_ ) - 1 ) ? 0 : ii + 1 ] );
    }

    C_stroke::clear_all();
}

std::string
C_stroke_history::dump_stroke_buffer()
{
    std::string output;

    output = "\n\n";
    output += "  steno         translation       flag  sq  s'ceded  space\n";
    output += "  ------------  ----------------  ----  --  -------  ------\n";

    //log_writeln( C_log::LL_INFO, LOG_SOURCE, "dump_stroke_buffer(): 1" );

    C_stroke * stroke = stroke_curr_;

    for ( std::size_t ii = 0; ii < NUMBEROF( strokes_ ); ii++ )
    {
        output += ( stroke == stroke_curr_ ) ? "* " : "  ";

        std::string dmp_translation;

        //log_writeln( C_log::LL_INFO, LOG_SOURCE, "dump_stroke_buffer(): 2" );
        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  stroke->translation: %p", stroke->translation );s

        if ( strlen( stroke->translation ) > 0 )
        {
            dmp_translation = std::string( "|" ) + ctrl_to_text( stroke->translation ) + std::string( "|" );
        }

        char line[ 2048 ];

        snprintf( line, sizeof( line ), "%-12.12s  %-16.16s  %04x  %2d  %-7.7s  %-6.6s\n"
                                      , stroke->steno.c_str()
                                      , dmp_translation.c_str()
                                      , *stroke->flags
                                      , stroke->stroke_seqnum
                                      , stroke->superceded ? "true" : "false"
                                      , ( stroke->st == SP_NONE ) ? "none" : ( ( stroke->st == SP_BEFORE ) ? "before" : "after" ) );
        
        log_writeln( C_log::LL_INFO, LOG_SOURCE, "dump_stroke_buffer(): 3" );
        
        output += line;
        stroke = stroke->prev;
    }

    //log_writeln( C_log::LL_INFO, LOG_SOURCE, "dump_stroke_buffer(): 4" );

    return output;
}



// ------------------


// Returns true if a translation was made
bool
C_stroke_history::lookup( const std::string & steno, std::string & output, uint16_t & flags, uint16_t & flags_prev )
{
    // debug
    if ( steno == "#S" )
    {
        toggle_space_mode();
        return false;
    }

    // debug
    if ( steno == "#-D" )
    {
        output = dump_stroke_buffer();
        return true;
    }

    if ( steno == "*" )
    {
        output = undo();
    }
    else
    {
        stroke_curr_->find_best_match( steno, output, flags, flags_prev );
    }

    return true;
}

void
C_stroke_history::toggle_space_mode()
{
    space_mode_ = ( space_mode_ == SP_BEFORE ) ? SP_AFTER : SP_BEFORE;

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%s space mode active", ( space_mode_ == SP_BEFORE ) ? "Leading" : "Trailing" );

    clear_all_strokes();
}



// For example, "\n" becomes [0a]"
std::string
C_stroke_history::ctrl_to_text( const std::string & text )
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

const char *   C_stroke_history::NO_TRANSLATION = "";
const uint16_t C_stroke_history::NO_FLAGS       = 0x0000;
const uint16_t C_stroke_history::DUMMY_ATTACH   = ATTACH_TO_NEXT;

}
