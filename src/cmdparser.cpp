

#include <cstdint>
#include <cstring>

#include "cmdparser.h"
#include "cmdparserstate.h"
#include "miscellaneous.h"

using namespace stenosys;

namespace stenosys
{

C_cmd_parser::C_cmd_parser()
    : state_( C_st_init::s.instance() )
{
}

void
C_cmd_parser::set_state( std::shared_ptr< C_state > state )
{
    state_ = state;
}

void
C_cmd_parser::parse( const std::string & input, std::string & output, uint16_t & flags )
{
    input_ = input;

    set_state( C_st_init::s.instance() );

    do
    {
        state_->handler( this );

        delay( 500 );

    } while ( ! state_->done() );

    output = output_;
    flags  = flags_;
}

}
