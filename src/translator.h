// translator.h
#pragma once

#include <string>
#include <memory>

#include "dictionary.h"
//#include "history.h"
#include "formatter.h"

using namespace stenosys;

namespace stenosys
{

enum space_type { SP_NONE, SP_BEFORE, SP_AFTER };

const uint8_t STROKE_BUFFER_MAX = 10U;

class C_translator
{

public:

    C_translator( space_type trans_mode );
    ~C_translator();

    bool
    initialise( const std::string dictionary_path );

    bool
    translate( const std::string & steno, std::string & output );
    
private:
    
    C_translator(){}

    std::string 
    undo();

//    std::string
//    format_output( C_stroke * stroke_previous_to_best_match, uint8_t backspaces, C_stroke * current_stroke );

    void
    toggle_space_mode();

    std::string
    ctrl_to_text( const std::string & text );

private:
    
    space_type space_mode_;

    //std::unique_ptr< C_formatter > formatter_;

    
    static const char   * NO_TRANSLATION;
    static const uint16_t NO_FLAGS;
    static const uint16_t DUMMY_ATTACH;
};

}
