
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

bool
C_stroke::initialise()
{
    // Create a doubly-linked list of C_stroke objects
    C_stroke::curr_        = new C_stroke();
    C_stroke * stroke_prev = C_stroke::curr_;

    for ( std::size_t ii = 1; ii < STROKE_BUFFER_MAX; ii++ )
    {
        C_stroke * stroke_new = new C_stroke();

        stroke_prev->next_ = stroke_new;
        stroke_new->prev_  = stroke_prev;
        
        stroke_prev = stroke_new;
    }
    
    // Complete the circular buffer
    stroke_prev->next_     = C_stroke::curr_;
    C_stroke::curr_->prev_ = stroke_prev;

    return true;
}

void
C_stroke::clear_all()
{
    for ( size_t ii = 0; ii < STROKE_BUFFER_MAX; ii++ )
    {
        C_stroke::curr_->clear( curr_ );
        C_stroke::curr_= curr_->next_;
    }
}

void
C_stroke::clear( C_stroke * stroke )
{
    stroke->steno_         = "";
    stroke->found_         = false;
    stroke->best_match_    = false;
    stroke->translation_   = "";
    stroke->flags_         = 0;
    stroke->stroke_seqnum_ = 0;
    stroke->superceded_    = false;
}

// Find the match dictionary match, given a new steno chord and with reference to
// previous steno entries. The calling code can format the output text, using the
// flags from the current and previous stroke as required.
void
C_stroke::find_best_match( std::unique_ptr< C_dictionary > & dictionary
                         , const std::string &             steno
                         , std::string &                   text 
                         , uint16_t                        flags
                         , uint16_t                        flags_prev )
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

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "level: %u, steno_key: %s", level, steno_key.c_str()  );
    
    if ( ( stroke->steno_.length() == 0 ) || ( level >= LOOKBACK_MAX ) )
    {
        return;
    }

    std::string key = ( level == 0 ) ? steno_key : stroke->steno_ + std::string( "/" ) + steno_key;

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "key: %s", key.c_str()  );
    
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

std::string
C_stroke::undo( C_stroke ** stroke, space_type space_mode )
{
    std::string output;
#if 0
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
#endif
    return output;
}


std::string
C_stroke::dump()
{
    std::string output;

#if 0
    output = "\n\n";
    output += "  steno         translation       flag  sq  s'ceded  space\n";
    output += "  ------------  ----------------  ----  --  -------  ------\n";

    //log_writeln( C_log::LL_INFO, LOG_SOURCE, "dump_stroke_buffer(): 1" );

    C_stroke * stroke = curr_;

    for ( std::size_t ii = 0; ii < STROKE_BUFFER_MAX; ii++ )
    {
        output += ( ii == 0 ) ? "* " : "  ";

        std::string dmp_translation;

        //log_writeln( C_log::LL_INFO, LOG_SOURCE, "dump_stroke_buffer(): 2" );
        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  stroke->translation: %p", stroke->translation );s

        if ( strlen( stroke->translation_ ) > 0 )
        {
            dmp_translation = std::string( "|" ) + ctrl_to_text( stroke->translation_ ) + std::string( "|" );
        }

        char line[ 2048 ];

        snprintf( line, sizeof( line ), "%-12.12s  %-16.16s  %04x  %2d  %-7.7s  %-6.6s\n"
                                      , stroke->steno_.c_str()
                                      , dmp_translation.c_str()
                                      , *stroke->flags_
                                      , stroke->stroke_seqnum_
                                      , stroke->superceded_ ? "true" : "false" );
        
        log_writeln( C_log::LL_INFO, LOG_SOURCE, "dump_stroke_buffer(): 3" );
        
        output += line;
        stroke = stroke->prev;
    }

    //log_writeln( C_log::LL_INFO, LOG_SOURCE, "dump_stroke_buffer(): 4" );
#endif
    return output;
}

C_stroke * C_stroke::curr_ = nullptr;

}
