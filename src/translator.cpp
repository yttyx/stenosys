
#include <algorithm>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

//#include <common.h>
#include "dictionary.h"
#include "log.h"
#include "stenoflags.h"
#include "translator.h"

#define LOG_SOURCE "TRAN "

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_translator::C_translator( space_type space_mode )
    : space_mode_( space_mode )
{
    strokes_     = std::make_unique< C_strokes >();
    dictionary_  = std::make_unique< C_dictionary >();
}

C_translator::~C_translator()
{
}

bool
C_translator::initialise( const std::string & dictionary_path )
{
    return strokes_->initialise() && dictionary_->read( dictionary_path );
}

// Returns true if a translation was made
void
C_translator::translate( const std::string & steno, std::string & output )
{
    // debug
    if ( steno == "#S" )
    {
        toggle_space_mode();
        return;
    }

    // debug
    if ( steno == "#-D" )
    {
        output = strokes_->dump();
        return;
    }

#if 0
    if ( steno == "*" )
    {
        output = undo();
        return;
    }
#endif  

    uint16_t flags      = 0;
    uint16_t flags_prev = 0;
    bool     extends    = false;

    strokes_->find_best_match( dictionary_, steno, output, flags, flags_prev, extends );

    std::string translation = format( output, flags, flags_prev, extends );

    strokes_->set_translation( translation );
    
    output = translation;
}

// backspaces: Number of backspaces required to delete text already output
//             to get to the start point of outputting the current translation
std::string
C_translator::format( const std::string text
                    , uint16_t          flags
                    , uint16_t          flags_prev 
                    , bool              extends )
{
    std::string output;

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
            /* output += ' '; */
            /* current_stroke->st = SP_BEFORE; */

            log_writeln( C_log::LL_INFO, LOG_SOURCE, "Added leading space" );
        }

        output += formatted;

        // Only output a trailing space if the stroke generated some text
        if ( config_space_after && ( ! attach_to_next ) && ( formatted.length() > 0 ) )
        {
            output += ' ';
        }
    }
    
    return formatted;
}

/*
std::string
C_translator::undo()
{
    if ( stroke_curr_->steno.length() == 0 )
    {
        // Nothing to undo
        log_writeln( C_log::LL_INFO, LOG_SOURCE, "Nothing to undo" );
        return "";
    }
    
    std::string output;

    uint16_t backspaces = 0;
    bool     got_translation = ( strlen( stroke_curr_->translation ) > 0 );

    if ( got_translation )
    {
        backspaces = ( uint8_t ) ( strlen( stroke_curr_->translation ) + ( ( stroke_curr_->st != SP_NONE ) ? 1 : 0 ) );
    }
    else
    {
        backspaces = ( uint8_t ) stroke_curr_->steno.length();
    }

    output += std::string( backspaces, '\b' );

    if ( got_translation )
    {
        C_stroke * stroke_prev = stroke_curr_->prev;

        if ( stroke_prev->st == SP_AFTER )
        {
            const uint16_t flags = *stroke_curr_->flags;

            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "UNDO: flags : %04x", current_stroke_flags );

            if ( flags & ATTACH_TO_PREVIOUS )
            {
                // Restore the space that would previously have been removed when attaching the current stroke's output
                output += " ";

                log_writeln( C_log::LL_INFO, LOG_SOURCE, "UNDO: Restored space" );
            }
        }
    }

    C_stroke * stroke_lookback  = stroke_curr_;

    // Position to the first stroke in the stroke sequence
    while ( stroke_lookback->stroke_seqnum > 1 )
    {
        stroke_lookback = stroke_lookback->prev;
    }

    // Replay any previous strokes in the sequence
    while ( stroke_lookback != stroke_curr_ )
    {
        if ( stroke_lookback->superceded )
        {
            if ( stroke_lookback->st == SP_BEFORE )
            {
                output += " ";
            }

            if ( stroke_lookback->translation != nullptr )
            {
                output += stroke_lookback->translation;
            }
            else
            {
                output += stroke_lookback ->steno;
            }

            if ( stroke_lookback->st == SP_AFTER )
            {
                output += " ";
            }

            stroke_lookback->superceded = false;
        }

        stroke_lookback = stroke_lookback->next;
    }

    clear( stroke_curr_ );

    stroke_curr_ = stroke_curr_->prev;

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "UNDO: output: |%s|", ctrl_to_text( output ).c_str() );

    return output;
}
*/

void
C_translator::toggle_space_mode()
{
    space_mode_ = ( space_mode_ == SP_BEFORE ) ? SP_AFTER : SP_BEFORE;

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%s space mode active", ( space_mode_ == SP_BEFORE ) ? "Leading" : "Trailing" );

//    clear_all_strokes();
}

}
