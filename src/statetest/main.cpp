#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "basestate.h"
#include "test.h"

using namespace stenosys;


int main( int argc, char *argv[] )
{
    C_test test;

    test.change_state_to( C_state_A::s.instance() );

    while ( ! test.run() )
    {
    }

    return 0;
}
