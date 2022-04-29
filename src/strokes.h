// stroke.h
#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include "history.h"
#include "stroke.h"

using namespace stenosys;

namespace stenosys
{

#define STROKE_BUFFER_MAX 12
#define LOOKBACK_MAX      6

class C_strokes
{

public:

    C_strokes();
    ~C_strokes();

    bool
    initialise();

    void
    add( const std::string & steno );

    bool
    get_current( std::string & steno );
    
    bool 
    get_previous( std::string & steno );

    void 
    clear();
    
private:

    std::string steno_curr_;

    std::unique_ptr< C_history< std::string, 10 > > stroke_history_;
};

}
