#include <cstring>
#include <stdio.h>

#include "state.h"
#include "cmdparserstate.h"

using namespace  stenosys;


namespace stenosys
{

void
C_state_A::handler( C_cmd_parser * p )
{
    //fprintf( stdout, "C_state_A::handler()\n" );

    p->str().replace( 0, 5, "Howdo" );
    
    set_state( p, C_state_B::s.instance(), "C_state_B" );
}

void
C_state_B::handler( C_cmd_parser * p )
{
    //fprintf( stdout, "C_state_B::handler()\n" );
   
    p->str().replace( 6, 5, "Matey" );

    set_state( p, C_state_C::s.instance(), "C_state_C" );
}

void
C_state_C::handler( C_cmd_parser * p )
{
    //fprintf( stdout, "C_state_C::handler()\n" );
    
    p->str().replace( 11, 1, "?" );
    
    set_state( p, C_state_D::s.instance(), "C_state_D" );
}

void
C_state_D::handler( C_cmd_parser * p )
{
    //fprintf( stdout, "C_state_D::handler()\n" );
    
    C_state::done_ = true;
}

}
