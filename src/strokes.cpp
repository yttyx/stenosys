
#include <algorithm>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "common.h"
#include "dictionary.h"
#include "log.h"
#include "stenoflags.h"
#include "strokes.h"

#define LOG_SOURCE "STRKS"

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_strokes::C_strokes( const C_dictionary  & dictionary )
    : dictionary_( dictionary )
    , best_match_level_( 0 )
{
    history_ = std::make_unique< C_history< C_stroke , 10 > >();
}

C_strokes::~C_strokes()
{
}

bool
C_strokes::initialise()
{
    return true;
}

// Add a steno stroke and find the best match
void
C_strokes::find_best_match( const std::string & steno
                          , std::string &       text 
                          , uint16_t &          flags
                          , uint16_t &          flags_prev
                          , bool &              extends )
{

    //// Move to new stroke and initialise it
    //stroke_curr_ = stroke_curr_->get_next();
    
    //stroke_curr_->clear();
    //stroke_curr_->set_steno( steno );

    //// Default to raw steno in case no matching steno entry found
    //stroke_curr_->set_translation( steno );

    best_match_level_ = 0;

    C_stroke best_match;
    
    uint16_t level = 0;

    find_best_match( level, steno, text, best_match );

    //if ( best_match != nullptr )
    //{
        //stroke_curr_->set_translation( text );
        //flags      = stroke_curr_->get_flags();
        //flags_prev = best_match->get_prev()->get_flags();
        //extends    = stroke_curr_->get_extends(); 
    //}
    //else
    //{
        //text       = stroke_curr_->get_translation();
        //flags      = 0x0000;
        //flags_prev = 0x0000;
        //extends    = false;
    //}
}

void
C_strokes::find_best_match( uint16_t            level
                          , const std::string & steno_key
                          , std::string &       text
                          , C_stroke &          best_match )
{

//    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "level: %u, steno_key: %s", level, steno_key.c_str() );
    
    //if ( ( stroke->get_steno().length() == 0 ) || ( level >= LOOKBACK_MAX ) )
    //{
        //return;
    //}

    //std::string key = ( level == 0 ) ? steno_key : stroke->get_steno() + std::string( "/" ) + steno_key;
    std::string key;

    //// Look up the stroke
    //uint16_t flags = 0;

    //if ( dictionary_.lookup( key, text, flags) )
    //{
        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "key %s FOUND, text is %s", key.c_str(), text.c_str() );
        //best_match        = stroke;
        //best_match_level_ = level;
        
        //stroke->set_seqnum( 0 );
    //}
    //else
    //{
        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "key %s NOT found, text is %s", key.c_str(), text.c_str() );
    //}

    //find_best_match( dictionary_, level + 1, stroke->get_prev(), key, text, best_match );

    log_writeln_fmt( C_log::LL_VERBOSE_1, LOG_SOURCE, "level: %u, best_match_level_: %u", level, best_match_level_ );

    if ( level < best_match_level_ )
    {
        //stroke->set_seqnum( stroke->get_prev()->get_seqnum() + 1 );
    }
}

void
C_strokes::set_translation( const std::string translation )
{
    //stroke_curr_->set_translation( translation );    
}
    
std::string
C_strokes::get_previous_translation()
{
    //return stroke_curr_->get_prev()->get_translation();
    return std::string( "" );
}


#if 0
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

void
C_strokes::clear()
{
    for ( size_t ii = 0; ii < STROKE_BUFFER_MAX; ii++ )
    {
        //stroke_curr_->clear();
        //stroke_curr_ = stroke_curr_->get_next();
    }
}

void
C_strokes::dump()
{
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "  steno         translation       flgs  sq  s'ceded" );
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "  ------------  ----------------  ----  --  -------" );

    //C_stroke * stroke = stroke_curr_;

    for ( std::size_t ii = 0; ii < STROKE_BUFFER_MAX; ii++ )
    {
        //std::string translation = stroke->get_translation();
        std::string translation;

        std::string trans_field;

        if ( translation.length() > 0 )
        {
            trans_field = std::string( "|" ) + ctrl_to_text( translation ) + std::string( "|" );
        }

        char line[ 2048 ];

        //snprintf( line, sizeof( line ), "%s%-12.12s  %-16.16s  %04x  %2d  %-7.7s"
                                      //, ( ii == 0 ) ? "* " : "  "
                                      //, stroke->get_steno().c_str()
                                      //, trans_field.c_str()
                                      //, stroke->get_flags()
                                      //, stroke->get_seqnum()
                                      //, stroke->get_superceded() ? "true" : "false" );
   
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%s", line );

        //stroke = stroke->get_prev();
    }
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
