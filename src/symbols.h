#pragma once

#include <memory>
#include <stdio.h>

//using namespace stenosys;

namespace stenosys
{

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

};

}
