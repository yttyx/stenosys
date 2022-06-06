#pragma once

#include <cstdint>
#include <string>
#include <memory>

//#include "cmdparserstate.h"
#include "state.h"

using namespace stenosys;

namespace stenosys
{

class C_cmd_parser
{

public:

    C_cmd_parser( const std::string & str );
    ~C_cmd_parser(){}

    void
    set_state( std::shared_ptr< C_state > state );

    bool
    run();

    void
    str( std::string & str );

    std::string &
    str();

private:
    
    std::shared_ptr< C_state > state_;

    std::string  str_;

};

}
