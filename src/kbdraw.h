//kbdraw.h
#pragma once

#include <linux/types.h>
#include <memory>
#include <string>
#include <termios.h>

#include "buffer.h"
#include "geminipr.h"
#include "keyevent.h"
#include "mutex.h"
#include "thread.h"
#include "timer.h"

namespace stenosys
{

class C_kbd_raw : public C_thread
{

public:

    C_kbd_raw();
    ~C_kbd_raw();

    bool
    initialise( const std::string & device );
    
    bool
    start();

    void
    stop();

    bool
    read( key_event_t & key_event, uint8_t & scan_code );
    
    bool
    acquired();
    
private:

    bool 
    detect_keyboard( std::string & device );

    int
    open_keyboard( const std::string & device );

    void
    thread_handler();

    bool
    open();

    bool
    read();

private:
    
    bool        abort_;
    int         handle_;

    bool        acquired_;

    std::string device_;

    C_timer     timer_;

    std::unique_ptr< C_buffer< uint16_t, 256 > > buffer_;
};

}
