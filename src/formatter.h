// formatter.h
#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <memory>

#include "dictionary.h"
#include "history.h"
#include "stenoflags.h"
#include "strokes.h"

using namespace stenosys;

namespace stenosys
{

class C_formatter
{

public:

    C_formatter( space_type space_mode );
    ~C_formatter();

    std::string
    format( alphabet_type      alphabet_mode
           , const std::string latin
           , const std::string shavian
           , uint16_t          flags
           , uint16_t          flags_prev 
           , bool              extends );
    
    std::string
    transition_to( const std::string & prev, const std::string & curr, bool extends, bool undo );
    
private:
    
    C_formatter(){}

    int 
    find_point_of_difference( const std::string & s1, const std::string & s2 );

private:
    
    space_type space_mode_;

};

}
