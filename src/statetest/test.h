#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include "basestate.h"

namespace stenosys
{

class C_test
{

public:

    C_test( const std::string & str );
    ~C_test(){}

    void
    change_state_to( std::shared_ptr< C_base_state > state );

    bool
    run();

    void
    str( std::string & str );

    std::string &
    str();

private:
    
    std::shared_ptr< C_base_state > state_;

    std::string  str_;

};

}
