
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

void
C_stroke::clear()
{
    steno_         = "";
    found_         = false;
    best_match_    = false;
    best_match_    = false;
    translation_   = "";
    flags_         = 0;
    stroke_seqnum_ = 0;
    superceded_    = false;
}

void
C_stroke::set_next( C_stroke * next )
{
    next_ = next;
}

void
C_stroke::set_prev( C_stroke * prev )
{
    prev_ = prev;
}

void
C_stroke_history::C_stroke_history()
{
    // Turn the stroke array into a double-linked circular buffer
    for ( std::size_t ii = 0; ii < NUMBEROF( strokes_ ); ii++ )
    {
        C_stroke * stroke = &strokes_[ ii ];

        stroke->set_prev( &strokes_[ ( ii == 0 ) ? NUMBEROF( strokes_ ) - 1 : ii - 1 ] );
        stroke->set_next( &strokes_[ ( ii == NUMBEROF( strokes_ ) - 1 ) ? 0 : ii + 1 ] );
    }

    //clear_all_strokes();

    dictionary_ = std::make_unique< C_dictionary >();
}

bool
C_stroke_history::initialise( const std::string dictionary_path )
{
    return dictionary_->read( dictionary_path );
}

// Assumes the most recent stroke is pointed to by stroke_curr_.
// That stroke entry needs to be populated with the information required to render or undo it.
void
C_stroke_history::find_best_match( const std::string & steno, const std::string & steno_key, std::string & translation )
{
    //TBW End of find best match call sequence check

    //clear();

//    steno_ = steno;

    std::string new_steno_key;

    if ( steno_key.length() == 0 )
    {
        // Initial key
        new_steno_key = steno_key;
    }
    else
    {
        // Form a compound key
        new_steno_key = steno  + std::string( "/" ) + steno_key;
    }

    // Look up the stroke
    std::string text;
    uint16_t    flags = 0;

    if ( dictionary_->lookup( steno, text, flags) )
    {
//        found_       = true;
//        translation_ = translation;
//        flags_       = flags;
    }
    //TBW to be continued...

    //    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "translation: |%s|", stroke_curr_->translation );

    //    lookback_stroke = lookback_stroke->prev;
    //}    

    //uint8_t stroke_seqnum = 1;
    //uint8_t backspaces    = 0;

    //lookback_stroke = best_match_stroke;

    //// Fix up the stroke sequence number working forward from the best match stroke,
    //// calculating the number of backspaces required to remove the output from an
    //// earlier best match stroke as we go.
    //// The earlier best stroke output will be replaced by the output from the current
    //// stroke.
    //while ( lookback_stroke != stroke_curr_ )
    //{
    //    lookback_stroke->stroke_seqnum = stroke_seqnum++;

    //    if ( ! lookback_stroke->superceded )
    //    {
    //        backspaces += ( uint8_t ) ( strlen( lookback_stroke->translation ) + ( ( lookback_stroke->st == SP_NONE ) ? 1 : 0 ) );
    //        lookback_stroke->superceded = true;
    //    }

    //    lookback_stroke = lookback_stroke->next;
    //}

    //stroke_curr_->stroke_seqnum = stroke_seqnum;

    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "stroke_curr_->translation: %s",   stroke_curr_->translation );
    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "stroke_curr_->flag: %04x",        *stroke_curr_->flags );
    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "stroke_curr_->stroke_seqnum: %u", stroke_curr_->stroke_seqnum );

    // format_output( best_match_stroke->prev, backspaces, stroke_curr_ );
}

// ------------------


// Returns true if a translation was made
bool
C_stroke_history::lookup( const std::string & steno, std::string & output )
{
    // debug
    if ( steno == "#S" )
    {
//        toggle_space_mode();
        return false;
    }

    // debug
    if ( steno == "#-D" )
    {
 //       output = dump_stroke_buffer();
        return true;
    }

    if ( steno == "*" )
    {
//        output = undo();
    }
    else
    {
        stroke_curr_ = stroke_curr_->next;

        stroke_curr_.find_best_match( steno, "", translation );

        stroke_curr_->steno         = steno;
        stroke_curr_->stroke_seqnum = 1;
       
        output = lookup();
        output = formatter_->format( output );
    }    

    return true;
}

std::string
C_stroke_history::undo()
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

// backspaces: Number of backspaces required to delete text already output
//             to get to the start point of outputting the current translation
std::string
C_stroke_history::format_output( C_stroke * previous_stroke_best_match
                                 , uint8_t    backspaces
                                 , C_stroke * current_stroke )
{
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

// Format contents of stroke buffer into a string
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

void
C_stroke_history::toggle_space_mode()
{
    space_mode_ = ( space_mode_ == SP_BEFORE ) ? SP_AFTER : SP_BEFORE;

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%s space mode active", ( space_mode_ == SP_BEFORE ) ? "Leading" : "Trailing" );

    clear_all_strokes();
}

void
C_stroke_history::clear_all_strokes()
{
    for ( std::size_t ii = 0; ii < NUMBEROF( strokes_ ); ii++ )
    {
        &strokes_[ ii ].clear();
    }

    // Create a dummy entry for an attached stroke so the very first user
    // stroke does not have a leading space when leading spaces are
    // selected.
    strokes_[ 0 ].flags = &ATTACH_TO_NEXT;
}

// Convert control characters in a string to text
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
