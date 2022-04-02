// stenokeyboard.h
#pragma once

#include <linux/types.h>
#include <memory>
#include <string>
#include <termios.h>

#include "buffer.h"
#include "geminipr.h"
#include "mutex.h"
#include "thread.h"

#include "promicro.h"

#define BUFFER_SIZE 256

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
    read( uint16_t & key_code );
    
private:
    

    void
    thread_handler();

    bool
    get_byte( unsigned char & ch );

   bool
    allow_repeat( uint16_t key_code );

private:
    
    int  handle_;
    bool abort_;

    std::unique_ptr< C_buffer< uint16_t > > buffer_;
};

}
