
#include <algorithm>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "common.h"
#include "dictionary.h"
#include "log.h"
#include "stenoflags.h"
#include "stroke.h"

#define LOG_SOURCE "STROK"

using namespace stenosys;

namespace stenosys
{

extern C_log log;

void
C_stroke::set_next( C_stroke * stroke )
{
    next_ = stroke;
}

void
C_stroke::set_prev( C_stroke * stroke )
{
    prev_ = stroke;
}
    
C_stroke *
C_stroke::get_next()
{
    return next_;
}

C_stroke *
C_stroke::get_prev()
{
    return next_;
}

const std::string &
C_stroke::get_steno()
{
    return steno_;
}

const std::string &
C_stroke::get_translation()
{
    return translation_;
}


uint16_t
C_stroke::get_flags()
{
    return flags_;
}

uint16_t
C_stroke::get_seqnum()
{
    return seqnum_;
}

bool
C_stroke::get_superceded()
{
    return superceded_;
}

void
C_stroke::clear()
{
    steno_         = "";
    found_         = false;
    best_match_    = false;
    translation_   = "";
    flags_         = 0;
    seqnum_        = 0;
    superceded_    = false;
}

bool
C_strokes::initialise()
{
    // Create a doubly-linked list of C_stroke objects
    C_stroke * stroke_first = new C_stroke();
    C_stroke * stroke_prev = stroke_first;

    for ( std::size_t ii = 1; ii < STROKE_BUFFER_MAX; ii++ )
    {
        C_stroke * stroke_new = new C_stroke();

        stroke_prev->set_next( stroke_new );
        stroke_new->set_prev( stroke_prev );
        
        stroke_prev = stroke_new;
    }
    
    // Complete the circular buffer
    stroke_prev->set_next( stroke_first );
    stroke_first->set_prev( stroke_prev );

    curr_ = stroke_first;

    return true;
}

void
C_strokes::clear_all()
{
    for ( size_t ii = 0; ii < STROKE_BUFFER_MAX; ii++ )
    {
        curr_->clear();
        curr_ = curr_->get_next();
    }
}

#if 0
// previous steno entries. The calling code can format the output text, using the
// flags from the current and previous stroke as required.
void
C_stroke::find_best_match( std::unique_ptr< C_dictionary > & dictionary
                         , const std::string &             steno
                         , std::string &                   text 
                         , uint16_t &                      flags
                         , uint16_t &                      flags_prev )
{
    // Move onto next stroke and initialise it
    C_stroke::curr_ = C_stroke::curr_->next_;
    clear( C_stroke::curr_ );

    C_stroke::curr_->steno_       = steno;
    C_stroke::curr_->translation_ = steno; // Default to raw steno in case steno entry not found

    uint16_t level = 0;

    C_stroke * best_match = nullptr;

    find_best_match( dictionary, level, C_stroke::curr_, steno, text, &best_match );

    if ( best_match != nullptr )
    {
        C_stroke::curr_->translation_ = text;
        flags = best_match->flags_;
        flags_prev = best_match->prev_->flags_;
    }
}

void
C_stroke::find_best_match( std::unique_ptr< C_dictionary > & dictionary
                         , uint16_t                        level
                         , C_stroke *                      stroke
                         , const std::string &             steno_key
                         , std::string &                   text
                         , C_stroke **                     best_match )
{

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "level: %u, steno_key: %s", level, steno_key.c_str() );
    
    if ( ( stroke->steno_.length() == 0 ) || ( level >= LOOKBACK_MAX ) )
    {
        return;
    }

    std::string key = ( level == 0 ) ? steno_key : stroke->steno_ + std::string( "/" ) + steno_key;

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "key: %s", key.c_str() );
    
    // Look up the stroke
    uint16_t flags = 0;

    if ( dictionary->lookup( key, text, flags) )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "key %s FOUND, text is %s", key.c_str(), text.c_str() );
        *best_match = stroke;
    }
    else
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "key %s NOT found, text is %s", key.c_str(), text.c_str() );
    }

    find_best_match( dictionary, level + 1, stroke->prev_, key, text, best_match );
}

std::string
C_stroke::undo()
{
    std::string output;
    
    if ( ( *stroke )->steno_.length() == 0 )
    {
        // Nothing to undo
        log_writeln( C_log::LL_INFO, LOG_SOURCE, "Nothing to undo" );
        return "";
    }
    

    uint16_t backspaces = 0;
    bool     got_translation = ( ( *stroke )->translation_.length() > 0 );

    if ( got_translation )
    {
//        backspaces = ( uint8_t ) ( ( *stroke )->translation_.length() ) + ( ( space_mode != SP_NONE ) ? 1 : 0 ) );
    }
    else
    {
  //      backspaces = ( uint8_t ) ( *stroke )->steno_.length();
    }

    output += std::string( backspaces, '\b' );

    if ( got_translation )
    {
        C_stroke * stroke_prev = ( *stroke )->prev_;

        if ( space_mode == SP_AFTER )
        {
            const uint16_t current_stroke_flags = ( *stroke )->flags_;

            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "UNDO: current_stroke_flags : %04x", current_stroke_flags );

            if ( current_stroke_flags & ATTACH_TO_PREVIOUS )
            {
                // Restore the space that would previously have been removed when attaching the current stroke's output
                output += " ";

                log_writeln( C_log::LL_INFO, LOG_SOURCE, "UNDO: Restored space" );
            }
        }
    }

    C_stroke * stroke_lookback = *stroke;

    // Position to the first stroke in the stroke sequence
    while ( stroke_lookback->stroke_seqnum_ > 1 )
    {
        stroke_lookback = stroke_lookback->prev_;
    }

    // Replay any previous strokes in the sequence
    while ( stroke_lookback != *stroke )
    {
        if ( stroke_lookback->superceded_ )
        {
            if ( space_mode == SP_BEFORE )
            {
                output += " ";
            }

            if ( stroke_lookback->translation_.length() != 0 )
            {
                output += stroke_lookback->translation_;
            }
            else
            {
                output += stroke_lookback ->steno_;
            }

            if ( space_mode == SP_AFTER )
            {
                output += " ";
            }

            stroke_lookback->superceded_ = false;
        }

        stroke_lookback = stroke_lookback->next_;
    }

    clear( *stroke );

    *stroke = ( *stroke)->prev_;

//    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "UNDO: output: |%s|", ctrl_to_text( output ).c_str() );
    return output;
}

#endif

std::string
C_strokes::dump()
{
    std::string output;

    output = "\n\n";
    output += "  steno         translation       flgs  sq  s'ceded  \n";
    output += "  ------------  ----------------  ----  --  -------  \n";

    //log_writeln( C_log::LL_INFO, LOG_SOURCE, "dump_stroke_buffer(): 1" );

    C_stroke * stroke = curr_;

    for ( std::size_t ii = 0; ii < STROKE_BUFFER_MAX; ii++ )
    {
        output += ( ii == 0 ) ? "* " : "  ";

        std::string translation = stroke->get_translation();

        std::string trans_field;

        if ( translation.length() > 0 )
        {
            trans_field = std::string( "|" ) + ctrl_to_text( translation ) + std::string( "|" );
        }

        char line[ 2048 ];

        snprintf( line, sizeof( line ), "%-12.12s  %-16.16s  %04x  %2d  %-7.7s\n"
                                      , stroke->get_steno().c_str()
                                      , trans_field.c_str()
                                      , stroke->get_flags()
                                      , stroke->get_seqnum()
                                      , stroke->get_superceded() ? "true" : "false" );
        output += line;
        stroke = stroke->get_prev();
    }

    return output;
}

// Convert control characters in a string to text
// For example, "\n" becomes [0a]"
std::string
C_strokes::ctrl_to_text( const std::string & text )
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

}
