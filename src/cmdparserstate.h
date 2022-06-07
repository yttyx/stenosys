#pragma once

#include <memory>
#include <stdio.h>

#include "cmdparser.h"
#include "single.h"
#include "state.h"

namespace stenosys
{

class C_cmd_parser;

STATE_DECLARATION( C_state_A, C_cmd_parser );
STATE_DECLARATION( C_state_B, C_cmd_parser );
STATE_DECLARATION( C_state_C, C_cmd_parser );
STATE_DECLARATION( C_state_D, C_cmd_parser );

}
