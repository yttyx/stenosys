// translator.h
#pragma once

#include <algorithm>
#include <string>
#include <memory>

#include "dictionary.h"
#include "history.h"
#include "strokes.h"

using namespace stenosys;

namespace stenosys
{

enum space_type { SP_NONE, SP_BEFORE, SP_AFTER };

class C_translator
{

public:

    C_translator( space_type space_mode );
    ~C_translator();

    bool
    initialise( const std::string & dictionary_path );

    void 
    translate( const std::string & steno, std::string & output );
    
private:
    
    C_translator(){}

    std::string
    format( const std::string translation, uint16_t flags, uint16_t flags_prev, bool extends );

    void
    toggle_space_mode();

private:
    
    space_type space_mode_;

    std::unique_ptr< C_dictionary >              dictionary_;
    std::unique_ptr< C_history< C_stroke, 10 > > strokes_;

};

}
