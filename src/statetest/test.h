#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include "basestate.h"

namespace stenosys
{

class C_test
{
//friend class C_state_A;
//friend class C_state_B;

public:

    C_test();
    ~C_test(){}

    void
    change_state_to( std::shared_ptr< C_base_state > state );

private:
    
    std::shared_ptr< C_base_state > state_;

    std::string  str_;

};

}
