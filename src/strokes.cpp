
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

C_strokes::C_strokes( C_dictionary & dictionary )
    : dictionary_( dictionary )
{
    history_ = std::make_unique< C_history< C_stroke, 10 > >();
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
C_strokes::find_best_match( const std::string &               steno
                          , std::string &                     text 
                          , uint16_t &                        flags
                          , uint16_t &                        flags_prev
                          , bool &                            extends )
{
    C_stroke new_stroke( steno );

    history_->add( new_stroke );

    std::string key;

    text = steno;  // Default to the raw steno

    C_stroke * stroke = nullptr;

    do
    {
        key = ( key.length() == 0 ) ? steno : stroke->steno() + std::string( "/" ) + key;
        
        if ( dictionary_.lookup( key, text, flags) )
        {
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "key %s FOUND, text: %s, flags: %u", key.c_str(), text.c_str(), flags );

            history_->curr()->translation( text );
            history_->curr()->flags( flags );

            // Set best match so far
            history_->set_bookmark();
        }
        
    } while ( history_->go_back( stroke ) );

    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "bookmark: steno: %s, seqnum %u"
                                               //, history_->bookmark()->steno().c_str()
                                               //, history_->bookmark()->seqnum() );
    
    // Work forward from the history bookmark (best match) and fix up the stroke sequence numbers
  
    history_->goto_bookmark();
   
    //flags      = history_->curr()->flags();
    flags_prev = history_->bookmark_prev()->flags();

    uint16_t seqnum = 1;

    history_->bookmark()->seqnum( seqnum );
    
    while ( history_->go_forward( stroke ) )
    {
        stroke->seqnum( ++seqnum );

        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "go_forward: steno: %s, seqnum %u"
                                                   //, history_->lookback()->steno().c_str()
                                                   //, history_->lookback()->seqnum() );
    }

    extends = ( seqnum > 1 );
}

void
C_strokes::set_translation( const std::string translation )
{
    history_->curr()->translation( translation );    
}
    
std::string
C_strokes::get_previous_translation()
{
    return history_->prev()->translation();
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
C_strokes::dump()
{
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "steno         translation       flgs  sq  s'ceded" );
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "------------  ----------------  ----  --  -------" );


    history_->reset_lookback();

    C_stroke * stroke = history_->curr();

    do
    {
        std::string trans_field;
        std::string translation = stroke->translation();

        if ( translation.length() > 0 )
        {
            trans_field = std::string( "|" ) + ctrl_to_text( translation ) + std::string( "|" );
        }

        char line[ 2048 ];

        snprintf( line, sizeof( line ), "%-12.12s  %-16.16s  %04x  %2d"
                                      , stroke->steno().c_str()
                                      , trans_field.c_str()
                                      , stroke->flags()
                                      , stroke->seqnum() );
   
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%s", line );
    
    } while ( history_->go_back( stroke ) );
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
