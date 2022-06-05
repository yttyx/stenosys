#include <cstring>
#include <stdio.h>

#include "basestate.h"
#include "test.h"

using namespace  stenosys;

namespace stenosys
{

void
C_base_state::change_state_to( C_test * test, std::shared_ptr< C_base_state > state, const char * description )
{
    fprintf( stdout, "Change state to: %s", description );

    test->change_state_to( state );
}

void
C_base_state::handler( C_test * p )
{
    fprintf( stdout, "C_base_state::handler()" );
}

void
C_state_A::handler( C_test * p )
{
    fprintf( stdout, "C_state_A::handler()" );
}

}
