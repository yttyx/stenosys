

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "common.h"
#include "log.h"
#include "utf8.h"

#define LOG_SOURCE "STRK "

using namespace stenosys;

namespace stenosys
{

extern C_log log;


C_utf8::C_utf8( const std::string & str )
    : index_curr_( 0 )
{
}


}
