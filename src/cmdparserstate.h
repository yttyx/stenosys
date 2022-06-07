#pragma once

#include <memory>
#include <stdio.h>

#include "cmdparser.h"
#include "single.h"
#include "state.h"

namespace stenosys
{

class C_cmd_parser;

STATE_DECLARATION( C_st_init,         C_cmd_parser );
STATE_DECLARATION( C_st_find_command, C_cmd_parser );
STATE_DECLARATION( C_st_got_command,  C_cmd_parser );
STATE_DECLARATION( C_st_end,          C_cmd_parser );

}
