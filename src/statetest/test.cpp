

#include <cstdint>
#include <cstring>

#include "test.h"


using namespace stenosys;

namespace stenosys
{

C_test::C_test()
    : state_( C_state_A::s.instance() )
{

}

void
C_test::change_state_to( std::shared_ptr< C_base_state > state )
{
    state_ = state;
}

void
C_test::run()
{
    state_->handler( this );
}


}
