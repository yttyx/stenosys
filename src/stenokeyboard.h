// stenokeyboard.h
#pragma once

#include <linux/types.h>
#include <memory>
#include <string>
#include <termios.h>

#include "geminipr.h"
#include "kbdraw.h"
#include "kbdsteno.h"
#include "keyevent.h"

namespace stenosys
{

class C_steno_keyboard
{

public:

    C_steno_keyboard();

    ~C_steno_keyboard();

    bool
    initialise( const std::string & device_raw, const std::string & device_steno );

    bool
    start();

    void
    stop();

    bool
    read( key_event_t & key_event, uint8_t & scancode );
    
    bool
    read( S_geminipr_packet & packet );

private:
    
    std::unique_ptr< C_kbd_raw >   raw_;
    std::unique_ptr< C_kbd_steno > steno_;
};

}
