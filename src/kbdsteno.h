// kbdsteno.h
#pragma once

#include <linux/types.h>
#include <memory>
#include <string>
#include <termios.h>

#include "buffer.h"
#include "geminipr.h"
#include "mutex.h"
#include "thread.h"

namespace stenosys
{

class C_kbd_steno : public C_thread
{

public:

    C_kbd_steno();
    ~C_kbd_steno();

    bool
    initialise( const std::string & device );
    
    bool
    start();

    void
    stop();

    bool
    read( S_geminipr_packet & packet );

private:
    

    int
    set_interface_attributes( int fd, int speed );

    void
    thread_handler();

    bool
    get_byte( unsigned char & ch );

private:
    
    int  handle_;
    bool abort_;

    std::unique_ptr< C_buffer< S_geminipr_packet > > buffer_;
};

}
