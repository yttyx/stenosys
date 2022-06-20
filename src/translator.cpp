
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
#include "symbols.h"
#include "translator.h"

#define LOG_SOURCE "TRAN "

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_translator::C_translator( alphabet_type alphabet)
    : alphabet_( alphabet)
{
    dictionary_ = std::make_unique< C_dictionary >();
    symbols_    = std::make_unique< C_symbols >();
    strokes_    = std::make_unique< C_strokes >( *dictionary_.get(), *symbols_.get() );
    formatter_  = std::make_unique< C_formatter >();
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
}

void
C_translator::add_stroke( const std::string & steno, std::string & output )
{
    uint16_t flags_curr = 0;
    uint16_t flags_prev = 0;
    bool     extends    = false;

    std::string text;

    if ( steno.find( PUNCTUATION_STARTER ) == std::string::npos  )
    {
        // Normal stroke
        strokes_->add_stroke( steno, alphabet_, text, flags_curr, flags_prev, extends );
    }
    else
    {
        // Punctuation stroke
        strokes_->add_stroke( steno, text, flags_curr, flags_prev );
    }

    std::string curr = formatter_->format( alphabet_, text, flags_curr, flags_prev, extends );

    strokes_->translation( curr );

    std::string prev = strokes_->previous_translation();

    output = formatter_->transition_to( prev, curr, flags_curr, flags_prev, extends, false );
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

        output = formatter_->transition_to( prev, curr, flags_curr, flags_prev, extends, true );
    }

    strokes_->undo();
}

void
C_translator::toggle_alphabet_mode()
{
    alphabet_= ( alphabet_== AT_LATIN ) ? AT_SHAVIAN : AT_LATIN;

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%s alphabet active", ( alphabet_== AT_LATIN ) ? "Latin" : "Shavian" );
}

}
