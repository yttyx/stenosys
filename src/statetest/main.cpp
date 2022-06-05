#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "basestate.h"
#include "test.h"

using namespace stenosys;


int main( int argc, char *argv[] )
{
    C_test test( "Hello World!" );

    fprintf( stdout, "%s\n", test.str().c_str() );
    
    test.set_state( C_state_A::s.instance() );

    while ( ! test.run() )
    {
        fprintf( stdout, "%s\n", test.str().c_str() );
    }


    return 0;
}
