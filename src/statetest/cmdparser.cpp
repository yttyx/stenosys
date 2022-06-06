

#include <cstdint>
#include <cstring>

#include "cmdparser.h"
#include "cmdparserstate.h"

using namespace stenosys;

namespace stenosys
{

C_cmd_parser::C_cmd_parser( const std::string & str )
    : state_( C_state_A::s.instance() )
{
    str_ = str;
}

void
C_cmd_parser::set_state( std::shared_ptr< C_state > state )
{
    state_ = state;
}

bool
C_cmd_parser::run()
{
    state_->handler( this );

    return state_->done();
}

void
C_cmd_parser::str( std::string & str )
{
    str_ = str;
}

std::string &
C_cmd_parser::str()
{
    return str_;
}

}
