

#include <cstdint>
#include <cstring>

#include "test.h"


using namespace stenosys;

namespace stenosys
{

C_test::C_test( const std::string & str )
    : state_( C_state_A::s.instance() )
{
    str_ = str;
}

void
C_test::set_state( std::shared_ptr< C_base_state > state )
{
    state_ = state;
}

bool
C_test::run()
{
    state_->handler( this );

    return state_->done();
}

void
C_test::str( std::string & str )
{
    str_ = str;
}

std::string &
C_test::str()
{
    return str_;
}

}
