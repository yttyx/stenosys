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

    C_test();
    ~C_test(){}

    void
    change_state_to( std::shared_ptr< C_base_state > state );

    void
    run();

private:
    
    std::shared_ptr< C_base_state > state_;

    std::string  str_;

};

}
