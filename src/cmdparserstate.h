#pragma once

#include <memory>
#include <stdio.h>

#include "cmdparser.h"
#include "single.h"
#include "state.h"

namespace stenosys
{

class C_cmd_parser;

STATE_DECLARATION( C_st_init,            C_cmd_parser );
STATE_DECLARATION( C_st_in_text     ,    C_cmd_parser );
STATE_DECLARATION( C_st_escaped_char,    C_cmd_parser );
STATE_DECLARATION( C_st_raw_command,     C_cmd_parser );
STATE_DECLARATION( C_st_got_command,     C_cmd_parser );
STATE_DECLARATION( C_st_got_command_2,   C_cmd_parser );
STATE_DECLARATION( C_st_get_command_end, C_cmd_parser );
STATE_DECLARATION( C_st_end,             C_cmd_parser );

}
