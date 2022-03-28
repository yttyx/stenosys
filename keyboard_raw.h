// keyboard_raw.h
#pragma once

#include <linux/types.h>
#include <string>
#include <termios.h>

#include "mutex.h"
#include "thread.h"

#include "pro_micro.h"

#define BUFFER_SIZE 256

namespace stenosys
{

class C_keyboard_raw : public C_thread
{

public:

    C_keyboard_raw();
    ~C_keyboard_raw();

    bool
    initialise( const std::string & device );
    
    bool
    start();

    bool
    read( __u16 & key_code );

    void
    stop();

private:
    
    void
    thread_handler();

    bool
    allow_repeat( __u16 key_code );

    void
    buffer_put( __u16 key_event, __u16 key_code );

private:
    
    int  handle_;
    bool abort_;

    __u16 buffer_[ BUFFER_SIZE ];

    int  buffer_put_index_;
    int  buffer_get_index_;
    int  buffer_count_;

    static unsigned char keytable[];

    C_mutex  buffer_lock_;

};

}
