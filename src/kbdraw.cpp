// kbdraw.cpp
// Class for inputting keypresses from a keyboard, typically running the QMK firmware
// This class supports:
// - Keypresses direct from the keyboard (normal typing, USB HID)

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

#define LOG_SOURCE "KBRAW"
#define DEV_DIR    "/dev/input/"
#define EVENT_DEV  "event"
#define PLANCK_DEV "Planck"

using namespace stenosys;

namespace stenosys
{

extern C_log log;

// Input: device: path of keyboard device e.g. "/dev/input/event3"
//                or "" for auto-detection
C_kbd_raw::C_kbd_raw()
    : abort_( false )
    , handle_( -1 )
    , acquired_( false )
{
    buffer_ = std::make_unique< C_buffer< uint16_t, 256 > >();
    
    timer_.stop();
}

C_kbd_raw::~C_kbd_raw()
{
    if ( handle_ >= 0 )
    {
        // Release grab
        ioctl( handle_, EVIOCGRAB, ( void * ) 0 );
        
        ::close( handle_ );
        log_writeln( C_log::LL_INFO, LOG_SOURCE, "Closed raw keyboard device" );
    }
}

// -----------------------------------------------------------------------------------
// Foreground thread code
// -----------------------------------------------------------------------------------

bool
C_kbd_raw::initialise( const std::string & device )
{
    device_ = device;
    handle_ = -1;

    return true;
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

// Returns true if device has been successfully opened either at program run
// time, or when access to the device has been lost and re-aquired.
bool
C_kbd_raw::acquired()
{
    bool result = acquired_;

    if ( acquired_ )
    {
        acquired_ = false;
    }

    return result;
}

// -----------------------------------------------------------------------------------
// Background thread code
// -----------------------------------------------------------------------------------

enum eThreadState
{
    tsAwaitingOpen
,   tsOpenSuccessful
,   tsReading
,   tsReadError
,   tsWaitBeforeReopenAttempt
};

void
C_kbd_raw::thread_handler()
{
    eThreadState thread_state = tsAwaitingOpen;
    
    while ( ! abort_ )
    {
        switch ( thread_state )
        {
            case tsAwaitingOpen:
                
                thread_state = open() ? tsOpenSuccessful : tsWaitBeforeReopenAttempt;
                break;

            case tsOpenSuccessful:

                acquired_ = true;
                thread_state = tsReading;

                log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Using raw keyboard device %s", device_in_use_.c_str() );
                break;

            case tsReading:

                thread_state = read() ? tsReading : tsReadError;
                break;
        
            case tsReadError:

                timer_.start( 5000 );
                thread_state = tsWaitBeforeReopenAttempt;
                log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Lost access to raw keyboard device %s", device_in_use_.c_str() );
                break;
        
            case tsWaitBeforeReopenAttempt:
                
                if ( timer_.expired() )
                {
                    thread_state = tsAwaitingOpen;
                }
                else
                {
                    delay( 10 );
                }
                break;
        }
    }

    log_writeln( C_log::LL_INFO, LOG_SOURCE, "Shutting down raw keyboard thread" );
}
    
bool
C_kbd_raw::open( void )
{
    if ( device_.length() == 0 )
    {
        std::string detected_device;

        if ( detect_keyboard( detected_device ) )
        {
            device_in_use_ = detected_device;

            handle_ = open_keyboard( device_in_use_ );
        }
    }
    else
    {
        handle_ = open_keyboard( device_ );
    }

    return handle_ >= 0;
}


bool
C_kbd_raw::read( void )
{ 
    struct input_event kbd_event[ 64 ];

    int bytes_read = ::read( handle_, kbd_event, sizeof( struct input_event ) * 64 );

    log_writeln_fmt( C_log::LL_VERBOSE_1, LOG_SOURCE, "thread_handler, bytes_read: %d", bytes_read );

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
                
                return true;
            }
        }
    }
    else if ( bytes_read < 0 )
    {
        // Read error, most likely because device has become inaccessible
        // (for example, when using a KVM switch to switch from one PC to 
        // another). We will close the device and trust that the caller
        // will attempt to reopen it. 
        // Release grab
        ioctl( handle_, EVIOCGRAB, ( void * ) 0 );
        
        ::close( handle_ );
        handle_ = -1;
        return false;
    } 

    return true;
}

bool
C_kbd_raw::detect_keyboard( std::string & device )  
{
    int device_event_num = 99999999;

    // Try to auto-detect
    struct dirent * dir_entry = nullptr;

    DIR * dir = opendir( DEV_DIR );

    if ( dir != nullptr )
    {
        while ( ( dir_entry = readdir( dir ) ) != nullptr )
        {
            //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "device: %s", dir_entry->d_name );

            if ( strstr( dir_entry->d_name, EVENT_DEV ) != nullptr )
            {   
                std::string dev_path = std::string( DEV_DIR ) + dir_entry->d_name;

                int hnd = -1;

                // Open device
                if ( ( hnd = ::open( dev_path.c_str(), O_RDONLY ) ) < 0 )
                {
                    continue;
                }

                char name[ 256 ];
                
                int rc = -1;
                
                if ( ( rc = ioctl( hnd, EVIOCGNAME( sizeof( name ) ), name ) ) < 0 )
                {
                    log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "ioctl: EVIOCGNAME failed, rc = %d, errno = %d", rc, errno );
                } 

                close( hnd );

                //log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "device name: %s", name );
                
                if ( strstr( name, PLANCK_DEV ) != nullptr )
                {
                    // Found a Planck keyboard entry, but there will be two entries for the one keyboard, and we need
                    // the lowest numbered one to be able to successfully grab it (TODO: find a better way of deciding
                    // between the two).
            
                    //log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "Got a planck entry on %s", dir_entry->d_name );
                    
                    int num = atoi( dir_entry->d_name + strlen( EVENT_DEV ) );

                    //log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "num: %d", num );
                    
                    if ( num < device_event_num )
                    {
                        device_event_num = num;
                    }
                }
            }
        }
    }

    closedir( dir );

    if ( device_event_num < 99999999 )
    {
        device = DEV_DIR + std::string( EVENT_DEV ) + std::to_string( device_event_num );
                
        return true;
    }

    return false;
}

int
C_kbd_raw::open_keyboard( const std::string & device )
{
    int hnd = -1;

    // Open device
    if ( ( hnd = ::open( device.c_str(), O_RDONLY ) ) < 0 )
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "Failed to open raw keyboard device %s", device.c_str() );
        return -1;
    }

    int version = 0;

    // Get device version
    if ( ioctl( hnd, EVIOCGVERSION, &version ) )
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "Failed to get device version: %s", device.c_str() );
        close( hnd );
        return -1;
    }

    log_writeln_fmt( C_log::LL_VERBOSE_1, LOG_SOURCE, "  Driver version is %d.%d.%d", version >> 16, ( version >> 8 ) & 0xff, version & 0xff );
   
    struct input_id id;

    // Get device information
    ioctl( hnd, EVIOCGID, &id );

    log_writeln_fmt( C_log::LL_VERBOSE_1, LOG_SOURCE, "  Input device id: bus 0x%x vendor 0x%x product 0x%x version 0x%x"
                                                , id.bustype
                                                , id.vendor
                                                , id.product
                                                , id.version );
    int rc = -1;
    
    // Try to get device for exclusive use, so only we get the keyboard events, not the
    // linux kernel as well.
    if ( ( rc = ioctl( hnd, EVIOCGRAB, ( void * ) 1 ) ) < 0 )
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "  ioctl: EVIOCGRAB failed, rc = %d, errno = %d", rc, errno );
        
        close( hnd );
        return -1;
    }

    return hnd;
}

}
