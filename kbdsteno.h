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

class C_steno_keyboard : public C_thread
{

public:

    C_steno_keyboard();
    ~C_steno_keyboard();

    bool
    initialise( const std::string & device );
    
    bool
    initialise( const std::string & device_raw, const std::string & device_steno );

    bool
    start();

    void
    stop();

    bool
    read_raw( uint16_t & key_code );
    
    bool
    read_steno( S_geminipr_packet & packet );

private:
    
    bool
    initialise_raw( const std::string & device );
    
    bool
    initialise_steno( const std::string & device );

    int
    set_interface_attributes( int fd, int speed );

    void
    thread_handler();

    void
    raw_handler();

    void
    steno_handler();

    bool
    get_byte( unsigned char & ch );

   bool
    allow_repeat( uint16_t key_code );

private:
    
    int  handle_;
    bool abort_;

    std::unique_ptr< C_buffer< uint16_t > >          raw_buffer_;
    std::unique_ptr< C_buffer< S_geminipr_packet > > steno_buffer_;
};

}
