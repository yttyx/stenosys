
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
    dictionary_  = std::make_unique< C_dictionary >();
}

C_translator::~C_translator()
{
}

bool
C_translator::initialise( const std::string & dictionary_path )
{
    return dictionary_->read( dictionary_path );
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
        //strokes_->dump();
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

    // strokes_->find_best_match( dictionary_, steno, output, flags, flags_prev, extends );

    std::string translation = format( output, flags, flags_prev, extends );

    // strokes_->set_translation( translation );
    
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

// Add a steno stroke and find the best match
void
C_strokes::find_best_match( std::unique_ptr< C_dictionary > & dictionary
                          , const std::string &               steno
                          , std::string &                     text 
                          , uint16_t &                        flags
                          , uint16_t &                        flags_prev
                          , bool &                            extends )
{
    // Move to new stroke and initialise it
    stroke_curr_ = stroke_curr_->get_next();
    
    stroke_curr_->clear();
    stroke_curr_->set_steno( steno );

    // Default to raw steno in case no matching steno entry found
    stroke_curr_->set_translation( steno );

    best_match_level_ = 0;

    uint16_t   level      = 0;
    C_stroke * best_match = nullptr;

    find_best_match( dictionary, level, stroke_curr_, steno, text, best_match );

    if ( best_match != nullptr )
    {
        stroke_curr_->set_translation( text );
        flags      = stroke_curr_->get_flags();
        flags_prev = best_match->get_prev()->get_flags();
        extends    = stroke_curr_->get_extends(); 
    }
    else
    {
        text       = stroke_curr_->get_translation();
        flags      = 0x0000;
        flags_prev = 0x0000;
        extends    = false;
    }
}

void
C_strokes::find_best_match( std::unique_ptr< C_dictionary > & dictionary
                          , uint16_t                          level
                          , C_stroke *                        stroke
                          , const std::string &               steno_key
                          , std::string &                     text
                          , C_stroke * &                      best_match )
{

//    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "level: %u, steno_key: %s", level, steno_key.c_str() );
    
    if ( ( stroke->get_steno().length() == 0 ) || ( level >= LOOKBACK_MAX ) )
    {
        return;
    }

    std::string key = ( level == 0 ) ? steno_key : stroke->get_steno() + std::string( "/" ) + steno_key;

    // Look up the stroke
    uint16_t flags = 0;

    if ( dictionary->lookup( key, text, flags) )
    {
//        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "key %s FOUND, text is %s", key.c_str(), text.c_str() );
        best_match        = stroke;
        best_match_level_ = level;
        
        stroke->set_seqnum( 0 );
    }
    else
    {
//        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "key %s NOT found, text is %s", key.c_str(), text.c_str() );
    }

    find_best_match( dictionary, level + 1, stroke->get_prev(), key, text, best_match );

    log_writeln_fmt( C_log::LL_VERBOSE_1, LOG_SOURCE, "level: %u, best_match_level_: %u", level, best_match_level_ );

    if ( level < best_match_level_ )
    {
        stroke->set_seqnum( stroke->get_prev()->get_seqnum() + 1 );
    }
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
