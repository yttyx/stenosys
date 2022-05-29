#pragma once

namespace stenosys
{

#define KC_EXECUTE 0x86    // Used to switch between Latin and Shavian alphbets
                           // when typing. According to the QMK source code
                           // KC_EXECUTE is defined as 0x74, but in Linux the
                           // scancode comes through as 0x86. 

enum key_event_t { KEY_EV_UNKNOWN, KEY_EV_UP, KEY_EV_DOWN, KEY_EV_AUTO };

}
