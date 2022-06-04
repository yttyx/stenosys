
#include <algorithm>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

//#include <common.h>
#include "dictionary.h"
#include "formatter.h"
#include "log.h"
#include "miscellaneous.h"
#include "stenoflags.h"
#include "strokes.h"
#include "translator.h"

#define LOG_SOURCE "TRAN "

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_translator::C_translator( alphabet_type alphabet_mode, space_type space_mode )
    : alphabet_mode_( alphabet_mode )
    , space_mode_( space_mode )
{
    dictionary_ = std::make_unique< C_dictionary >();
    strokes_    = std::make_unique< C_strokes >( *dictionary_.get() );
    formatter_  = std::make_unique< C_formatter >( space_mode );
}

C_translator::~C_translator()
{
}

bool
C_translator::initialise( const std::string & dictionary_path )
{
    return strokes_->initialise() && dictionary_->read( dictionary_path );
}

// Returns true if a translation was made
void
C_translator::translate( const std::string & steno, std::string & output )
{
    if ( steno[ 0 ] == '#' )
    {
        if ( steno == "#A" )
        {
            toggle_alphabet_mode();
        }
        else if ( steno == "#S" )
        {
            toggle_space_mode();
        }
        else if ( steno == "#-D" )
        {
            strokes_->dump();
        }
    }
    else
    {
        if ( steno == "*" )
        {
            undo_stroke( output );
        }
        else
        {
            add_stroke( steno, output );
        }
    }

    log_writeln( C_log::LL_INFO, LOG_SOURCE, "C_translator::translate()" );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  steno : %s", steno.c_str() );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  output: >%s<", ctrl_to_text( output ).c_str() );

}

void
C_translator::add_stroke( const std::string & steno, std::string & output )
{
    uint16_t flags_curr = 0;
    uint16_t flags_prev = 0;
    bool     extends    = false;

    std::string latin;
    std::string shavian;

    strokes_->find_best_match( steno, latin, shavian, flags_curr, flags_prev, extends );

    std::string curr = formatter_->format( alphabet_mode_, latin, shavian, flags_curr, flags_prev, extends );

    strokes_->translation( curr );

    std::string prev = strokes_->previous_translation();

    log_writeln( C_log::LL_INFO, LOG_SOURCE, "add_stroke()" );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  prev: >>%s<<", ctrl_to_text( prev ).c_str() );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  curr: >>%s<<", ctrl_to_text( curr ).c_str() );

    output = formatter_->transition_to( prev, curr, flags_curr, flags_prev, extends, false );
    
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  output: >>%s<<", ctrl_to_text( output ).c_str() );
}

void
C_translator::undo_stroke( std::string & output )
{
    std::string curr = strokes_->translation();

    if ( curr.length() > 0 )
    {
        std::string prev = strokes_->previous_translation();

        bool extends = strokes_->extends();

        uint16_t flags_curr = strokes_->flags();
        uint16_t flags_prev = strokes_->flags_prev();

        log_writeln( C_log::LL_INFO, LOG_SOURCE, "undo_stroke()" );
        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  prev: >>%s<<", ctrl_to_text( prev ).c_str() );
        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  curr: >>%s<<", ctrl_to_text( curr ).c_str() );
    
        output = formatter_->transition_to( prev, curr, flags_curr, flags_prev, extends, true );
    
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  output: >>%s<<", ctrl_to_text( output ).c_str() );
    }

    strokes_->undo();
}

void
C_translator::toggle_space_mode()
{
    space_mode_ = ( space_mode_ == SP_BEFORE ) ? SP_AFTER : SP_BEFORE;

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%s space mode active", ( space_mode_ == SP_BEFORE ) ? "Leading" : "Trailing" );
}

void
C_translator::toggle_alphabet_mode()
{
    alphabet_mode_ = ( alphabet_mode_ == AT_LATIN ) ? AT_SHAVIAN : AT_LATIN;

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%s alphabet active", ( alphabet_mode_ == AT_LATIN ) ? "Latin" : "Shavian" );
}

}
