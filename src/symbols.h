#pragma once

#include "utf8.h"
#include <string>
#include <stdio.h>

#include <memory>
#include <unordered_map>

//using namespace stenosys;

namespace stenosys
{

#define PUNCTUATION_STARTER  "SKWH"
#define PUNCTUATION_VARIANTS "FRPBLG"

class C_symbols
{
public:

    C_symbols();
    
    virtual ~
    C_symbols() {}

    bool
    lookup( const std::string & steno, std::string & text, uint16_t & flags );

protected:

private:

    std::unique_ptr< std::unordered_map< std::string, std::string > > symbol_map_;

};

}
