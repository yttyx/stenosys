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
    fprintf( stdout, "Change state to: %s\n", description );

    test->change_state_to( state );
}

void
C_base_state::handler( C_test * p )
{
    fprintf( stdout, "C_base_state::handler()\n" );
}

void
C_state_A::handler( C_test * p )
{
    fprintf( stdout, "C_state_A::handler()\n" );

    change_state_to( p, C_state_B::s.instance(), "C_state_B" );
}

void
C_state_B::handler( C_test * p )
{
    fprintf( stdout, "C_state_B::handler()\n" );
    
    change_state_to( p, C_state_C::s.instance(), "C_state_C" );
}

void
C_state_C::handler( C_test * p )
{
    fprintf( stdout, "C_state_C::handler()\n" );

    change_state_to( p, C_state_A::s.instance(), "C_state_A" );
}

}
