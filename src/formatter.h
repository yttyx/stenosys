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

    C_formatter();
    ~C_formatter();

    std::string
    format( alphabet_type      alphabet_mode
           , const std::string latin
           , const std::string shavian
           , uint16_t          flags_curr
           , uint16_t          flags_prev 
           , bool              extends );
    
    std::string
    transition_to( const std::string & prev
                 , const std::string & curr
                 , uint16_t            flags_curr
                 , uint16_t            flags_prev 
                 , bool                extends
                 , bool                undo );
    
private:
    
    int 
    find_point_of_difference( const std::string & s1, const std::string & s2 );

    bool
    attach( uint16_t flags_prev, uint16_t flags_curr );

private:

};

}
