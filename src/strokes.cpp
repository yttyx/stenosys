
#include <algorithm>

//TEMP for dict lookup tests
#include <chrono>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "dictionary_i.h"
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

C_strokes::C_strokes( C_symbols & symbols )
    : symbols_( symbols )
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

    //TEMP
    //word_check( "being" );
    //word_check( "orang" );
    //word_check( "abn" );

    return true;
}

//TEMP
//void
//C_strokes::word_check( const std::string & word )
//{
    ////TEMP Test speed of brute-force word lookup
    //std::list< std::string > word_list;

    //auto start = std::chrono::steady_clock::now();

    //word_lookup( word, 20, word_list );

    //for ( std::string entry : word_list )
    //{
        //fprintf( stdout, "  %s\n", entry.c_str() );
    //}

    //auto end = std::chrono::steady_clock::now();

    //auto duration_ms = std::chrono::duration_cast< std::chrono::milliseconds >( end - start ).count();

    //fprintf( stdout, "Elapsed: %lu mS\n", ( long int ) duration_ms );
    //fprintf( stdout, "----\n" );
//}

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
     
        std::string roman;
        std::string shavian;

        // Do dictionary lookup
        if ( lookup( key, alphabet, text, flags ) )
        {
            history_->curr()->translation( text );
            history_->curr()->flags( flags );

            // Set best match so far
            history_->set_bookmark();
        }

    } while ( history_->go_back( stroke ) );

    // Work forward from the history bookmark (best match) and fix up the stroke sequence numbers
    history_->goto_bookmark();
   
    flags_prev = history_->bookmark_prev()->flags();

    uint16_t seqnum = 1;

    history_->bookmark()->seqnum( seqnum );
    
    while ( history_->go_forward( stroke ) )
    {
        stroke->seqnum( ++seqnum );
    }

    extends = history_->curr()->extends();
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
    symbols_.lookup( steno, text, flags );
    
    C_stroke new_stroke( steno );

    new_stroke.flags( flags );
    new_stroke.translation( text );
    new_stroke.seqnum( 1 );
    
    history_->add( new_stroke );
    
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

// Clear all strokes
void
C_strokes::clear()
{
    while ( ( history_->curr()->steno().length() > 0 ) )
    {
        history_->curr()->clear();
        history_->remove();
    }
}


// Output: text and flags are only set if the dictionary entry is found
bool
C_strokes::lookup( const std::string & steno
                 , alphabet_type       alphabet
                 , std::string &       text
                 , uint16_t &          flags )
{
    const uint16_t * roman_flags   = nullptr;
    const uint16_t * shavian_flags = nullptr;

    const char * roman   = nullptr;
    const char * shavian = nullptr;

    // Look up entry in hashed dictionary
    if ( dictionary_lookup( steno.c_str(), roman, roman_flags, shavian, shavian_flags ) )
    {
        // If configured for Shavian, use the Shavian entry if it's not empty; otherwise use
        // the Roman alphabet entry.
        text  = ( alphabet == AT_SHAVIAN ) ? ( ( strlen( shavian ) > 0 ) ? shavian : roman ) : roman;
        flags = ( alphabet == AT_SHAVIAN ) ? *shavian_flags : *roman_flags;
    
        return true;
    }

    return false;
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
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "steno         translation                     flag  sn" );
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "------------  ------------------------------  ----  --" );

    history_->reset_lookback();

    C_stroke * stroke = history_->curr();

    do
    {
        std::string trans_field;
        std::string formatted;
        
        std::string translation = stroke->translation();
        
        int formatted_length = 0;

        formatted_length = ctrl_to_text( translation, formatted );

        trans_field = std::string( "|" ) + formatted + std::string( "<" );

        char line[ 2048 ];

        snprintf( line, sizeof( line ), "%-12.12s  %-s%*s  %04x  %2d"
                                      , stroke->steno().c_str()
                                      , trans_field.c_str()
                                      , 28 - formatted_length, ""
                                      , stroke->flags()
                                      , stroke->seqnum() );
   
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%s", line );
    
    } while ( history_->go_back( stroke ) );
}

}
