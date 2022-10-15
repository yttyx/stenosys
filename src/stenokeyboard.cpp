// stenokeyboard.cpp
// Class for inputting keypresses from a keyboard, typically running the QMK firmware
// This class supports:
// - Keypresses direct from the keyboard (normal typing, USB HID)
// - Steno packets in GeminiPR format (steno layer enabled on the keyboard, serial over USB)
//
#include <iostream>

#include <istream>
#include <memory.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "geminipr.h"
#include "kbdraw.h"
#include "kbdsteno.h"
#include "keyevent.h"
#include "log.h"
//#include "promicro.h"
#include "stenokeyboard.h"

#define LOG_SOURCE "STKBD"

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_steno_keyboard::C_steno_keyboard()
{
    raw_   = std::make_unique< C_kbd_raw >();
    steno_ = std::make_unique< C_kbd_steno>();
}

C_steno_keyboard::~C_steno_keyboard()
{
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "C_steno_keyboard destructor" );
}

// -----------------------------------------------------------------------------------
// Foreground thread code
// -----------------------------------------------------------------------------------

bool
C_steno_keyboard::initialise( const std::string & device_raw, const std::string & device_steno )
{
    return raw_->initialise( device_raw) && steno_->initialise( device_steno );
}

bool
C_steno_keyboard::start()
{
    return raw_->start() && steno_->start();
}

void
C_steno_keyboard::stop()
{
    raw_->stop();
    steno_->stop();
}

bool
C_steno_keyboard::read( key_event_t & key_event, uint8_t & scancode )
{
    return raw_->read( key_event, scancode );
}
    
bool
C_steno_keyboard::read( S_geminipr_packet & packet  )
{
    return steno_->read( packet );
}

}
