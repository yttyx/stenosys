
#include <algorithm>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "common.h"
#include "dictionary.h"
#include "log.h"
#include "miscellaneous.h"
#include "stenoflags.h"
#include "strokes.h"
#include "symbols.h"

#define LOG_SOURCE "STRKS"

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_strokes::C_strokes( C_dictionary & dictionary, C_symbols & symbols )
    : dictionary_( dictionary )
    , symbols_( symbols )
{
    history_ = std::make_unique< C_history< C_stroke, 10 > >();
}
    
C_strokes::~C_strokes()
{
}

bool
C_strokes::initialise()
{
    // Add a dummy stroke
   
    C_stroke new_stroke( "" );

    new_stroke.flags( ATTACH_TO_NEXT );

    history_->add( new_stroke );

    return true;
}

// Add a steno stroke and look back through the stroke history
// to find the best dictionary match.
void
C_strokes::add_stroke( const std::string & steno
                     , alphabet_type       alphabet
                     , std::string &       text
                     , uint16_t &          flags
                     , uint16_t &          flags_prev
                     , bool &              extends )
{
    C_stroke new_stroke( steno );

    history_->add( new_stroke );

    std::string key;

    text = steno;  // Default to the raw steno

    C_stroke * stroke = nullptr;

    do
    {
        key = ( key.length() == 0 ) ? steno : stroke->steno() + std::string( "/" ) + key;
     
        std::string shavian;

        if ( dictionary_.lookup( key, alphabet, text, flags) )
        {
            //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "key %s FOUND, text: %s, flags: %u"
                                                       //, key.c_str()
                                                       //, output.c_str()
                                                       //, flags );

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

    extends = history_->curr()->extends();
    
    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "history_->curr()->extends(): %d"
                                               //, history_->curr()->extends() );
}

// Add symbol stroke and process the chord in code instead
// of doing a dictionary lookup, to find the selected
// punctuation or symbol.
void
C_strokes::add_stroke( const std::string & steno
                     , std::string &       text
                     , uint16_t &          flags
                     , uint16_t &          flags_prev )
{
    C_stroke new_stroke( steno );

    new_stroke.seqnum( 1 );
    
    history_->add( new_stroke );
    
    symbols_.lookup( steno, text, flags );
    
    flags_prev = history_->bookmark_prev()->flags();
}

void
C_strokes::undo()
{
    if ( ( history_->curr()->steno().length() > 0 ) )
    {
        history_->curr()->clear();
        history_->remove();
    }
}

void
C_strokes::translation( const std::string translation )
{
    history_->curr()->translation( translation );    
}

std::string &
C_strokes::translation()
{
    return history_->curr()->translation();    
}
    
std::string
C_strokes::previous_translation()
{
    return history_->prev()->translation();
}

uint16_t
C_strokes::flags()
{
    return history_->curr()->flags();
}

uint16_t
C_strokes::flags_prev()
{
    return history_->prev()->flags();
}

bool
C_strokes::extends()
{
    return history_->curr()->extends();
}

void
C_strokes::dump()
{
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "steno         translation       flgs  sn" );
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "------------  ----------------  ----  --" );

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

}
