// formatter.h
#pragma once

#include <algorithm>
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
    
private:
    
    C_formatter(){}

private:
    
    space_type space_mode_;

};

}
