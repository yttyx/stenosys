// translator.h
#pragma once

#include <algorithm>
#include <string>
#include <memory>

#include "dictionary.h"
#include "formatter.h"
#include "history.h"
#include "strokes.h"

using namespace stenosys;

namespace stenosys
{

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

    void
    add_stroke( const std::string & steno, std::string & output );

    void
    undo_stroke( std::string & output );

    void
    toggle_space_mode();

private:
    
    space_type space_mode_;

    std::unique_ptr< C_dictionary > dictionary_;
    std::unique_ptr< C_strokes >    strokes_;
    std::unique_ptr< C_formatter >  formatter_;

};

}
