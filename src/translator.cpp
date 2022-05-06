
#include <algorithm>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

//#include <common.h>
#include "dictionary.h"
#include "formatter.h"
#include "log.h"
#include "stenoflags.h"
#include "strokes.h"
#include "translator.h"

#define LOG_SOURCE "TRAN "

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_translator::C_translator( space_type space_mode )
    : space_mode_( space_mode )
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
    return dictionary_->read( dictionary_path );
}

// Returns true if a translation was made
void
C_translator::translate( const std::string & steno, std::string & output )
{
    // debug
    if ( steno == "#S" )
    {
        toggle_space_mode();
        return;
    }

    // debug
    if ( steno == "#-D" )
    {
        strokes_->dump();
        return;
    }

#if 0
    if ( steno == "*" )
    {
        output = undo();
        return;
    }
#endif  

    uint16_t flags      = 0;
    uint16_t flags_prev = 0;
    bool     extends    = false;

    std::string text;

    strokes_->find_best_match( steno, text, flags, flags_prev, extends );

    std::string formatted_curr = formatter_->format( text, flags, flags_prev, extends );

    strokes_->set_translation( formatted_curr );
    
    if ( extends )
    {
        std::string formatted_prev = strokes_->get_previous_translation();

        output = formatter_->extend( formatted_prev, formatted_curr );
    }
    else
    {
        output = formatted_curr;
    }
}

void
C_translator::toggle_space_mode()
{
    space_mode_ = ( space_mode_ == SP_BEFORE ) ? SP_AFTER : SP_BEFORE;

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%s space mode active", ( space_mode_ == SP_BEFORE ) ? "Leading" : "Trailing" );

//    clear_all_strokes();
}

}
