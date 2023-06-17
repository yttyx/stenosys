#pragma once

#include <cstdint>

namespace stenosys
{

// Steno command flags
const uint16_t ATTACH_TO_PREVIOUS   = 0x0001;
const uint16_t ATTACH_TO_NEXT       = 0x0002;
const uint16_t CAPITALISE_NEXT      = 0x0004;
const uint16_t LOWERCASE_NEXT       = 0x0008;
const uint16_t CAPITALISE_LAST      = 0x0010;
const uint16_t LOWERCASE_LAST       = 0x0020;
const uint16_t UPPERCASE_NEXT_WORD  = 0x0040;
const uint16_t LOWERCASE_NEXT_WORD  = 0x0080;
const uint16_t UPPERCASE_LAST_WORD  = 0x0100;
const uint16_t LOWERCASE_LAST_WORD  = 0x0200;
const uint16_t GLUE                 = 0x0400;
const uint16_t NAMING_DOT           = 0x0800;   // Shavian only

enum space_type    { SP_NONE, SP_BEFORE, SP_AFTER };
enum alphabet_type { AT_LATIN, AT_SHAVIAN };

}
