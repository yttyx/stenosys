#pragma once

#include <cstdint>

namespace stenosys
{

// Steno command flags
const uint16_t ATTACH_TO_PREVIOUS       = 0x0001;
const uint16_t ATTACH_TO_NEXT           = 0x0002;
const uint16_t CAPITALISE_NEXT          = 0x0004;
const uint16_t LOWERCASE_NEXT           = 0x0008;
const uint16_t CAPITALISE_LAST          = 0x0010;
const uint16_t LOWERCASE_LAST           = 0x0020;
const uint16_t UPPERCASE_NEXT_WORD      = 0x0040;
const uint16_t LOWERCASE_NEXT_WORD      = 0x0080;
const uint16_t UPPERCASE_LAST_WORD      = 0x0100;
const uint16_t LOWERCASE_LAST_WORD      = 0x0200;
const uint16_t GLUE                     = 0x0400;

// Flags used internally to command parsing
const uint16_t EMIT_SPACE               = 0x0800;
const uint16_t RAW                      = 0x1000;
const uint16_t GOT_TEXT                 = 0x2000;
const uint16_t UNUSED_2                 = 0x4000;
const uint16_t UNUSED_3                 = 0x8000;

const uint16_t INTERNAL_FLAGS_MASK      = 0xf800;

enum space_type { SP_NONE, SP_BEFORE, SP_AFTER };

}
