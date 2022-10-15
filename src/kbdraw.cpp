// kbdraw.cpp
// Class for inputting keypresses from a keyboard, typically running the QMK firmware
// This class supports:
// - Keypresses direct from the keyboard (normal typing, USB HID)
// - Steno packets in GeminiPR format (steno layer enabled on the keyboard, serial over USB)
//
#include <iostream>

#include <istream>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <memory>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "kbdraw.h"
#include "keyevent.h"
#include "log.h"
#include "miscellaneous.h"

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS( x )    ( ( ( ( x ) - 1 ) / BITS_PER_LONG ) + 1 )

#define LOG_SOURCE "KBRAW"

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_kbd_raw::C_kbd_raw()
{
    handle_ = -1;
    abort_  = false;
    buffer_ = std::make_unique< C_buffer< uint16_t > >();
}

C_kbd_raw::~C_kbd_raw()
{
    if ( handle_ >= 0 )
    {
        // Release grab
        ioctl( handle_, EVIOCGRAB, ( void * ) 0 );
        
        close( handle_ );
        log_writeln( C_log::LL_INFO, LOG_SOURCE, "Closed raw keyboard device" );
    }
}

// -----------------------------------------------------------------------------------
// Foreground thread code
// -----------------------------------------------------------------------------------

// Input: device: path of keyboard device e.g. "/dev/input/event3"
//                or "" for auto-detection

bool
C_kbd_raw::initialise( const std::string & device )
{
    handle_ = -1;

    if ( device.length() == 0 )
    {
        // Try to auto-detect
        struct dirent * dir_entry = nullptr;

        DIR * dir = opendir( "/dev/input" );
        
        if ( dir != nullptr )
        {
            while ( ( dir_entry = readdir( dir ) ) != nullptr )
            {
                log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "device: %s", dir_entry->d_name );

                if ( strstr( dir_entry->d_name, "event" ) != nullptr )
                {
                    std::string dev_path = "/dev/input/";

                    dev_path += dir_entry->d_name;

                    handle_ = detect_keyboard( dev_path.c_str() );

                    if ( handle_ >= 0 )
                    {
                        break;
                    }
                }
            }
        
            closedir( dir );
        }
    }
    else
    {
        detect_keyboard( device.c_str() );
    }

    return handle_ >= 0;
}

// returns a file handle or -1 if device not found
int
C_kbd_raw::detect_keyboard( const char * device )
{
    int version = 0;
    int hnd     = -1;

    //unsigned short id[ 4 ];
    unsigned long  bit[ EV_MAX ][ NBITS( KEY_MAX ) ];

    // Open device
    if ( ( hnd = ::open( device, O_RDONLY ) ) < 0 )
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "Failed to open raw keyboard device: %s - use sudo?", device );
        return -1;
    }
    
    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Opened raw keyboard device %s", device );

    // Get device version
    if ( ioctl( hnd, EVIOCGVERSION, &version ) )
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "Failed to get device version: %s", device );
        close( hnd );
        return -1;
    }
    
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  Driver version is %d.%d.%d", version >> 16, ( version >> 8 ) & 0xff, version & 0xff );
   
    struct input_id id;

    // Get device information
    ioctl( hnd, EVIOCGID, id );

    log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "  Input device id: bus 0x%x vendor 0x%x product 0x%x version 0x%x"
                                                , id.bustype
                                                , id.vendor
                                                , id.product
                                                , id.version );

    int rc = -1;

    char name[ 256 ];

    if ( ( rc = ioctl( hnd, EVIOCGNAME( sizeof( name ) ), name ) ) > 0 )
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "  Device name: %s", name );
    }
    else 
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "ioctl: EVIOCGNAME failed, rc = %d, errno = %d", rc, errno );
    }

    memset( bit, 0, sizeof( bit ) );
    ioctl( handle_, EVIOCGBIT( 0, EV_MAX ), bit[ 0 ] );

    // Try to get device for exclusive use, so only we get the keyboard events, not the
    // linux kernel as well.
    //if ( ( rc = ioctl( handle_, EVIOCGRAB, ( void * ) 1 ) ) < 0 )
    //{
        //log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "  ioctl: EVIOCGRAB failed, rc = %d, errno = %d", rc, errno );
    //}

    return -1; //TEMP rc >= 0;
}

bool
C_kbd_raw::start()
{
    return thread_start();
}

void
C_kbd_raw::stop()
{
    abort_ = true;
    thread_await_exit();
}

bool
C_kbd_raw::read( key_event_t & key_event, uint8_t & scancode )
{
    uint16_t key_entry = 0;

    if ( buffer_->get( key_entry ) )
    {
        key_event = ( key_event_t ) ( key_entry >> 8 );
        scancode  = key_entry & 0xff;

        return true;
    }

    return false;
}

// -----------------------------------------------------------------------------------
// Background thread code
// -----------------------------------------------------------------------------------

void
C_kbd_raw::thread_handler()
{
    struct input_event kbd_event[ 64 ];
    
    while ( ! abort_ )
    {
        int bytes_read = ::read( handle_, kbd_event, sizeof( struct input_event ) * 64 );
    
//        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "thread_handler, bytes_read: %d", bytes_read );

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
                        // TODO? Suppress auto-repeat for keys such as Shift, Ctrl and Meta
                        // Key auto-repeat
                        buffer_->put( ( KEY_EV_AUTO << 8 ) + kbd_event[ ii ].code );

                        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "key auto kbd_event[ii].code: %u", kbd_event[ii].code );
                    }
                    else if ( kbd_event[ ii ].value == 1 )
                    {
                        // Key down
                        buffer_->put( ( KEY_EV_DOWN << 8 ) + kbd_event[ ii ].code );
                    }
                    else if ( kbd_event[ ii ].value == 0 )
                    {
                        // Key up
                        buffer_->put( ( KEY_EV_UP << 8 ) + kbd_event[ ii ].code );
                    }
                }
            }
        }
    }

    log_writeln( C_log::LL_INFO, LOG_SOURCE, "Shutting down raw keyboard thread" );
}

//bool
//C_kbd_raw::allow_repeat( __u16 key_code )
//{
    //switch ( key_code )
    //{
        //case ARDUINO_KEY_LEFT_SHIFT:
        //case ARDUINO_KEY_RIGHT_SHIFT:
        //case ARDUINO_KEY_LEFT_CTRL:
        //case ARDUINO_KEY_RIGHT_CTRL:
        //case ARDUINO_KEY_LEFT_ALT:
        //case ARDUINO_KEY_RIGHT_ALT:
        //case ARDUINO_KEY_LEFT_GUI:
        //case ARDUINO_KEY_CAPS_LOCK:
        //case ARDUINO_KEY_INSERT:
            //return false;
    //}

    //return true;
//}

}
