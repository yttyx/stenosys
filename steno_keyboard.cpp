// C_steno_keyboard.cpp
// Class for inputting keypresses from the steno keyboard when on the base, non-steno layer

#include <iostream>

#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <memory.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "log.h"
#include "steno_keyboard.h"
#include "pro_micro.h"

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS( x )    ( ( ( ( x ) - 1 ) / BITS_PER_LONG ) + 1 )
#define LOG_SOURCE    "KBRAW"

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_steno_keyboard::C_steno_keyboard()
{
    handle_           = -1;
    abort_            = false;

    raw_buffer_ = std::make_unique< C_buffer< uint16_t > >();
    steno_buffer_ = std::make_unique< C_buffer< uint8_t > >();
}

C_steno_keyboard::~C_steno_keyboard()
{
    if ( handle_ >= 0 )
    {
        close( handle_ );
        log_writeln( C_log::LL_INFO, LOG_SOURCE, "Closed raw keyboard device" );
    }
}

bool
C_steno_keyboard::initialise( const std::string & device )
{
    unsigned short id[ 4 ];
    unsigned long  bit[ EV_MAX ][ NBITS( KEY_MAX ) ];

    int version = 0;

    // Open device
    if ( ( handle_ = ::open( device.c_str(), O_RDONLY ) ) < 0 )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Failed to open raw keyboard device: %s - use sudo?", device.c_str() );
        return false;
    }
    
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "Raw keyboard device opened" );

    // Get device version
    if ( ioctl( handle_, EVIOCGVERSION, &version ) )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Failed to get device version: %s", device );
        close( handle_ );
        handle_ = -1;
        return false;
    }
    
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Input driver version is %d.%d.%d", version >> 16, ( version >> 8 ) & 0xff, version & 0xff );
    
    // Get device information
    ioctl( handle_, EVIOCGID, id );

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x"
                                               , id[ ID_BUS ], id[ ID_VENDOR ], id[ ID_PRODUCT ], id[ ID_VERSION ] );

    memset( bit, 0, sizeof( bit ) );
    ioctl( handle_, EVIOCGBIT( 0, EV_MAX ), bit[ 0 ] );

    return true;
}

bool
C_steno_keyboard::start()
{
    return thread_start();
}

void
C_steno_keyboard::stop()
{
    abort_ = true;

    thread_await_exit();
}

bool
C_steno_keyboard::read_raw( uint16_t & key_code )
{
    return raw_buffer_->get( key_code );
}

void
C_steno_keyboard::thread_handler()
{
    struct input_event kbd_event[ 64 ];

    while ( ! abort_ )
    {
        int bytes_read = ::read( handle_, kbd_event, sizeof( struct input_event ) * 64 );

        if ( bytes_read >= ( int ) sizeof( struct input_event ) )
        {
            for ( int ii = 0; ii < (int) ( bytes_read / sizeof( struct input_event ) ); ii++ )
            {
                // We have:
                //   kbd_event[ii].time        timeval: 16 bytes (8 bytes for seconds, 8 bytes for microseconds)
                //   kbd_event[ii].type        See input-event-codes.h
                //   kbd_event[ii].code        See input-event-codes.h
                //   kbd_event[ii].value       01 for keypress, 00 for release, 02 for autorepeat
        
                if ( kbd_event[ ii ].type == EV_KEY )
                {
                    if ( kbd_event[ ii ].value == 2 )
                    {
                        // Suppress auto-repeat for keys such as Shift, Ctrl and Meta
                        // TODO This did not stop the sticky keys dialog popping up in Windows.
                        //      Find out what key events are leading to that (it doesn't occur
                        //      when holding down Shift on a directly-connected keyboard).
                        if ( allow_repeat( kbd_event[ii].code ) )
                        {
                            // Key auto-repeat
                            raw_buffer_->put( ( EV_KEY_AUTO << 8 ) + kbd_event[ii].code );
                        }

                        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "key auto kbd_event[ii].code: %u", kbd_event[ii].code );
                    }
                    else if ( kbd_event[ ii ].value == 1 )
                    {
                        // Key down
                        raw_buffer_->put( ( EV_KEY_DOWN << 8 ) + kbd_event[ii].code );
                    }
                    else if ( kbd_event[ ii ].value == 0 )
                    {
                        // Key up
                        raw_buffer_->put( ( EV_KEY_UP << 8 ) + kbd_event[ii].code );
                    }
                }
            }
        }
        else
        {
            log_writeln( C_log::LL_INFO, LOG_SOURCE, "raw keyboard read error - closing down read thread" );
        }
    }
}

bool
C_steno_keyboard::allow_repeat( __u16 key_code )
{
    switch ( key_code )
    {
        case ARDUINO_KEY_LEFT_SHIFT:
        case ARDUINO_KEY_RIGHT_SHIFT:
        case ARDUINO_KEY_LEFT_CTRL:
        case ARDUINO_KEY_RIGHT_CTRL:
        case ARDUINO_KEY_LEFT_ALT:
        case ARDUINO_KEY_RIGHT_ALT:
        case ARDUINO_KEY_LEFT_GUI:
        case ARDUINO_KEY_CAPS_LOCK:
        case ARDUINO_KEY_INSERT:
            return false;
    }

    return true;
}

}
