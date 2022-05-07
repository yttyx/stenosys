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
    format( const std::string translation, uint16_t flags, uint16_t flags_prev, bool extends );
    
    std::string
    extend( const std::string & prev, const std::string & curr );
    
    std::string
    undo( const std::string & curr,  const std::string & prev );

private:
    
    C_formatter(){}

    uint16_t 
    find_point_of_difference( const std::string & s1, const std::string & s2 );

private:
    
    space_type space_mode_;

};

}
