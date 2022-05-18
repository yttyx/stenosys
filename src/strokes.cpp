
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
                          , std::string &                     shavian 
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
        
        if ( dictionary_.lookup( key, text, shavian, flags) )
        {
            //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "key %s FOUND, text: %s, shavian: %s, flags: %u"
                                                       //, key.c_str()
                                                       //, text.c_str()
                                                       //, shavian.c_str()
                                                       //, flags );

            history_->curr()->translation( text );
            history_->curr()->shavian( shavian );
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

    extends = history_->curr()->extends();
    
    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "history_->curr()->extends(): %d"
                                               //, history_->curr()->extends() );
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

bool
C_strokes::extends()
{
    return history_->curr()->extends();
}

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
