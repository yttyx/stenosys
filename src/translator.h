// translator.h
#pragma once

#include <algorithm>
#include <string>
#include <memory>

#include "dictionary.h"
#include "formatter.h"
#include "geminipr.h"
#include "history.h"
#include "stenoflags.h"
#include "strokes.h"
#include "symbols.h"

using namespace stenosys;

namespace stenosys
{

class C_translator
{

public:

    C_translator( alphabet_type alphabet);
    ~C_translator();

    bool
    initialise();

    void 
    translate( const S_geminipr_packet & steno_packet, std::string & output );

    bool
    paper_tape();

private:
    
    C_translator(){}

    void
    add_stroke( const std::string & steno, std::string & output );

    void
    undo_stroke( std::string & output );

    void
    toggle_alphabet_mode();

    void
    toggle_space_mode();
    
    void
    toggle_paper_mode();

private:
    
    alphabet_type alphabet_;
    space_type    space_mode_;
    bool          paper_tape_;

    std::unique_ptr< C_symbols >    symbols_;
    std::unique_ptr< C_strokes >    strokes_;
    std::unique_ptr< C_formatter >  formatter_;

};

}
