
#include <algorithm>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

//#include <common.h>
#include "dictionary.h"
#include "log.h"
#include "stenoflags.h"
#include "stroke.h"
#include "translator.h"

#define LOG_SOURCE "TRAN "

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_translator::C_translator( space_type space_mode )
    : space_mode_( space_mode )
{
    stroke_     = std::make_unique< C_stroke >();
    dictionary_ = std::make_unique< C_dictionary >();
}

C_translator::~C_translator()
{
}

bool
C_translator::initialise( const std::string & dictionary_path )
{
    return C_stroke::initialise() && dictionary_->read( dictionary_path );
}

// Returns true if a translation was made
bool
C_translator::translate( const std::string & steno, std::string & output )
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
        output = C_stroke::dump();
        return true;
    }

    if ( steno == "*" )
    {
//        output = undo();
    }
    else
    {
        uint16_t flags      = 0;
        uint16_t flags_prev = 0;

        C_stroke::find_best_match( dictionary_, steno, output, flags, flags_prev );
    }    

    return true;
}

// backspaces: Number of backspaces required to delete text already output
//             to get to the start point of outputting the current translation
#if 0
std::string
C_translator::format_output( C_stroke * previous_stroke_best_match
                           , uint8_t    backspaces
                           , C_stroke * current_stroke )
{
    std::string output;

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "backspaces: %u", ( uint16_t ) backspaces );

    bool config_space_before = ( space_mode_ == SP_BEFORE );
    bool config_space_after  = ! config_space_before;

    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "config_space_before: %s", config_space_before ? "true" : "false" );
    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "config_space_after : %s", config_space_after  ? "true" : "false" );
   
    // Locate the flags for the stroke preceding the first stroke of the current best match
    // so they can be used to help format the output; for example, to suppress the space between
    // the previous stroke and the current stroke.
    
    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "previous_stroke_best_match              : %p", previous_stroke_best_match );
    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "previous_stroke_best_match->translation : %p", previous_stroke_best_match->translation );
    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "current_stroke->translation             : %p", current_stroke->translation );

    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "previous_stroke_best_match->flags       : %p", previous_stroke_best_match->flags );
    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "current_stroke->flags                   : %p", current_stroke->flags );

    const uint16_t previous_stroke_flags = *previous_stroke_best_match->flags;    
    const uint16_t current_stroke_flags  = *current_stroke->flags;

    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "previous_stroke_flags: %04x", previous_stroke_flags );
    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "current_stroke_flags : %04x", current_stroke_flags );

    // Attach to the previous stroke's output?
    bool attach_to_previous = ( ( previous_stroke_flags   & ATTACH_TO_NEXT )     ||
                                ( current_stroke_flags    & ATTACH_TO_PREVIOUS ) ||
                                ( ( previous_stroke_flags & GLUE ) && ( current_stroke_flags & GLUE ) ) );

    // Attach to the next stroke's output?
    bool attach_to_next = ( ( current_stroke_flags & ATTACH_TO_NEXT ) || ( current_stroke_flags & GLUE ) ); 

    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "attach_to_previous : %s", attach_to_previous  ? "true" : "false" );
    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "attach_to_next     : %s", attach_to_next      ? "true" : "false" );

    std::string output;

    if ( current_stroke->found )
    {
        std::string translation = current_stroke->translation;

        if ( previous_stroke_flags & CAPITALISE_NEXT )
        {
            translation[ 0 ] = toupper( translation[ 0 ] );
        }
        else if ( previous_stroke_flags & LOWERCASE_NEXT )
        {
            translation[ 0 ] = tolower( translation[ 0 ] );
        }
        else if ( previous_stroke_flags & LOWERCASE_NEXT_WORD )
        {
            std::transform( translation.begin(), translation.end(), translation.begin(), ::tolower );
        }
        else if ( previous_stroke_flags & UPPERCASE_NEXT_WORD )
        {
            std::transform( translation.begin(), translation.end(), translation.begin(), ::toupper );
        }

        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "config_space_after                        : %s", config_space_after  ? "true" : "false" );
        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "attach_to_previous                        : %s", attach_to_previous  ? "true" : "false" );
        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "previous_stroke_best_match->st == SP_AFTER: %s", ( previous_stroke_best_match->st == SP_AFTER ) ? "true" : "false" );

        if ( backspaces > 0 )
        {
            output += std::string( backspaces, '\b' );
        }
        else
        {
            // If we need to attach to the output from the previous stroke, and we're configured for space-after,
            // and the previous stroke did output a trailing space, then emit a backspace to remove that space.
            if ( config_space_after && attach_to_previous && ( previous_stroke_best_match->st == SP_AFTER ) )
            {
                output += '\b';
            }
        }

        // Only output a leading space if the stroke generated some text
        if ( config_space_before && ( ! attach_to_previous ) && ( translation.length() > 0 ) )
        {
            output += ' ';
            current_stroke->st = SP_BEFORE;

            log_writeln( C_log::LL_INFO, LOG_SOURCE, "Added leading space" );
        }

        output += translation;

        // Only output a trailing space if the stroke generated some text
        if ( config_space_after && ( ! attach_to_next ) && ( translation.length() > 0 ) )
        {
            output += ' ';
            current_stroke->st = SP_AFTER;

            log_writeln( C_log::LL_INFO, LOG_SOURCE, "Added trailing space" );
        }

        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "output: |%s|", ctrl_to_text( output ).c_str() );
    }
    else
    {
        // If stroke was not found output the raw steno
        output = current_stroke->steno;
    }
    return output;
}
#endif

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
            const uint16_t current_stroke_flags = *stroke_curr_->flags;

            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "UNDO: current_stroke_flags : %04x", current_stroke_flags );

            if ( current_stroke_flags & ATTACH_TO_PREVIOUS )
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
