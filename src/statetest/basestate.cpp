#include <cstring>
#include <stdio.h>

#include "basestate.h"
#include "test.h"

using namespace  stenosys;


namespace stenosys
{

C_base_state::C_base_state()
{
}

void
C_base_state::set_state( C_test * test, std::shared_ptr< C_base_state > state, const char * description )
{
    //fprintf( stdout, "Change state to: %s\n", description );

    test->set_state( state );
}

void
C_base_state::handler( C_test * p )
{
    fprintf( stdout, "C_base_state::handler()\n" );
}

bool 
C_base_state::done_ = false;


void
C_state_A::handler( C_test * p )
{
    //fprintf( stdout, "C_state_A::handler()\n" );

    p->str().replace( 0, 5, "Howdo" );
    
    set_state( p, C_state_B::s.instance(), "C_state_B" );
}

void
C_state_B::handler( C_test * p )
{
    //fprintf( stdout, "C_state_B::handler()\n" );
   
    p->str().replace( 6, 5, "Matey" );

    set_state( p, C_state_C::s.instance(), "C_state_C" );
}

void
C_state_C::handler( C_test * p )
{
    //fprintf( stdout, "C_state_C::handler()\n" );
    
    p->str().replace( 11, 1, "?" );
    
    set_state( p, C_state_D::s.instance(), "C_state_D" );
}

void
C_state_D::handler( C_test * p )
{
    //fprintf( stdout, "C_state_D::handler()\n" );
    
    C_base_state::done_ = true;
}

}
