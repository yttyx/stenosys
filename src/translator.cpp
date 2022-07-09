
#include <algorithm>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "dictionary.h"
#include "formatter.h"
#include "log.h"
#include "miscellaneous.h"
#include "stenoflags.h"
#include "strokes.h"
#include "symbols.h"
#include "translator.h"

#define LOG_SOURCE "TRANS"


using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_translator::C_translator( alphabet_type alphabet)
    : alphabet_( alphabet)
    , space_mode_( SP_BEFORE )
    , paper_tape_( false )
{
    symbols_    = std::make_unique< C_symbols >();
    strokes_    = std::make_unique< C_strokes >( *symbols_.get() );
    formatter_  = std::make_unique< C_formatter >();
}

C_translator::~C_translator()
{
}

bool
C_translator::initialise( const std::string & dictionary_path )
{
    formatter_->space_mode( space_mode_ );

    return strokes_->initialise();
}

// Returns true if a translation was made
void
C_translator::translate( const S_geminipr_packet & steno_packet, std::string & output )
{
    output.clear();

    std::string steno = C_gemini_pr::decode( steno_packet );

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
        else if ( steno == "#P" )
        {
            toggle_paper_mode();
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

    if ( paper_tape_ )
    {
        log_writeln_fmt_raw( C_log::LL_INFO, "%s", C_gemini_pr::to_paper( steno_packet ).c_str() );
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

    log_writeln_fmt_raw( C_log::LL_INFO, "%s", ( alphabet_== AT_LATIN ) ? "Roman" : "Shavian" );
}

void
C_translator::toggle_space_mode()
{
    space_mode_= ( space_mode_ == SP_BEFORE ) ? SP_AFTER : SP_BEFORE;

    formatter_->space_mode( space_mode_ );

    // Clear all strokes to get a clean start (avoids issues with undoing strokes across
    // a space-before/space-after transition).
    strokes_->clear();

    log_writeln_fmt_raw( C_log::LL_INFO, "Space %s", ( space_mode_ == SP_BEFORE ) ? "before" : "after" );
}

void
C_translator::toggle_paper_mode()
{
    paper_tape_ = ! paper_tape_;
    
    log_writeln_fmt_raw( C_log::LL_INFO, "Paper %s", paper_tape_ ? "on" : "off" );
}

}
