#include <cstring>
#include <stdio.h>

#include "cmdparser.h"
#include "state.h"

using namespace  stenosys;

namespace stenosys
{

C_state::C_state()
{
}

void
C_state::set_state( C_cmd_parser * cmd_parser, std::shared_ptr< C_state > state, const char * description )
{
    fprintf( stdout, "state set to: %s\n", description );

    cmd_parser->set_state( state );
}

void
C_state::handler( C_cmd_parser * p )
{
    fprintf( stdout, "C_state::handler()\n" );
}

void
C_state::reset()
{
    done_ = false;
}

bool 
C_state::done_ = false;

}
