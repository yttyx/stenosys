// kbdsteno.cpp
// Class for inputting keypresses from a keyboard, typically running the QMK firmware
// This class supports:
// - Keypresses direct from the keyboard (normal typing, USB HID)
// - Steno packets in GeminiPR format (steno layer enabled on the keyboard, serial over USB)
//
#include <iostream>

#include <istream>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <memory.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "geminipr.h"
#include "kbdsteno.h"
#include "log.h"
#include "miscellaneous.h"
//#include "promicro.h"

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS( x )    ( ( ( ( x ) - 1 ) / BITS_PER_LONG ) + 1 )

#define LOG_SOURCE "STKBD"

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_kbd_steno::C_kbd_steno()
{
    handle_ = -1;
    abort_  = false;
    buffer_ = std::make_unique< C_buffer< S_geminipr_packet, 16 > >();
    timer_.stop();
}

C_kbd_steno::~C_kbd_steno()
{
    if ( handle_ >= 0 )
    {
        close( handle_ );
        log_writeln( C_log::LL_VERBOSE_1, LOG_SOURCE, "Closed steno keyboard device" );
    }
}

// -----------------------------------------------------------------------------------
// Foreground thread code
// -----------------------------------------------------------------------------------

bool
C_kbd_steno::initialise( const std::string & device )
{
    device_ = device;

    return true;
}

bool
C_kbd_steno::start()
{
    return thread_start();
}

void
C_kbd_steno::stop()
{
    abort_ = true;

    thread_await_exit();
}

bool
C_kbd_steno::read( S_geminipr_packet & packet )
{
    return buffer_->get( packet );
}

// See https://stackoverflow.com/questions/20154157/termios-vmin-vtime-and-blocking-non-blocking-read-operations

int
C_kbd_steno::set_interface_attributes( int fd, int speed )
{
    struct termios tty;

    if ( tcgetattr( fd, &tty ) < 0 )
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "tcgetattr() error: %s\n", strerror( errno ) );
        return -1;
    }

    cfsetospeed( &tty, ( speed_t ) speed );
    cfsetispeed( &tty, ( speed_t ) speed );

    tty.c_cflag |= (CLOCAL | CREAD);    // ignore modem controls
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;                 // 8-bit characters
    tty.c_cflag &= ~PARENB;             // no parity bit
    tty.c_cflag &= ~CSTOPB;             // only need 1 stop bit
    tty.c_cflag &= ~CRTSCTS;            // no hardware flowcontrol

    // Set for non-canonical mode
    tty.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON );
    tty.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
    tty.c_oflag &= ~OPOST;

    if ( tcsetattr( fd, TCSANOW, &tty ) != 0 )
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "tcgetattr() error: %s\n", strerror( errno ) );
        return -1;
    }
    return 0;
}

// -----------------------------------------------------------------------------------
// Background thread code
// -----------------------------------------------------------------------------------

void
C_kbd_steno::thread_handler()
{
    unsigned int  packet_count = 0;
    unsigned char b            = 0;
    eThreadState  thread_state = tsAwaitingOpen;

    S_geminipr_packet packet;

    while ( ! abort_ )
    {
        switch ( thread_state )
        {
            case tsAwaitingOpen:

                thread_state = open() ? tsOpenSuccessful : tsReadError;
                break;

            case tsOpenSuccessful:

                log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "Using steno serial device %s", device_.c_str() );
                thread_state = tsAwaitingPacketHeader;
                break;
            
            case tsAwaitingPacketHeader:

                if ( get_byte( thread_state, b ) )
                {
                    // Packet header has top bit set
                    if ( b & 0x80 )
                    {
                        packet[ packet_count++ ]= b;
                        thread_state = tsPacketBody;
                    }
                }
                break;
            
            case tsPacketBody:
            
                if ( get_byte( thread_state, b ) )
                {
                    // Rest of packet bytes must have the top bit as zero
                    if ( b & 0x80 )
                    {
                        packet_count = 0;
                        thread_state = tsAwaitingPacketHeader;
                        log_writeln( C_log::LL_ERROR, LOG_SOURCE, "Invalid steno packet" );
                    }
                    else
                    {
                        packet[ packet_count++ ]= b;
                        
                        if ( packet_count == BYTES_PER_STROKE )
                        {
                            // We have a complete packet, add it to buffer
                            buffer_->put( packet );

                            packet_count = 0;
                            thread_state = tsAwaitingPacketHeader;
                        }
                    }
                }
                break;

            case tsReadError:

                log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "Lost access to steno serial device %s", device_.c_str() );
                
                // Read error: close file and wait 10 seconds before attempting to re-open it
                if ( handle_ >= 0 )
                {
                    close( handle_ );
                    handle_ = -1;
                }
        
                timer_.start( 10000 );
                
                thread_state = tsWaitBeforeReopenAttempt;
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
}

bool
C_kbd_steno::open( void )
{
    handle_ = ::open( device_.c_str(), O_RDONLY | O_NONBLOCK );

    if ( handle_ < 0 )
    {               
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "**Error opening steno device %s: %s", device_.c_str(), strerror( errno ) );
    }
    else
    {
        if ( set_interface_attributes( handle_, B19200 ) > -1 )
        {
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Steno device %s opened", device_.c_str() );
            return true;
        }
    }

    return false;
}

// Fetch a block of data in GeminiPR format
// returns: 1 if character was read, and ch is set to character
//          0 if no character available
//         -1 if read error
bool
C_kbd_steno::get_byte( eThreadState & state, unsigned char & ch )
{
    unsigned char buf[ 2 ];

    int res = ::read( handle_, buf, 1 );

    if ( res > 0 )
    {
        ch = buf[ 0 ];
        return true;
    }
    else if ( ( res < 0 ) &&  ( errno == EAGAIN ) )
    {
        // No data available: short sleep so we don't consume excessive CPU in a tight loop
        delay( 10 );
    }
    else
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "**Steno read error on serial device %s: %s"
                       , device_.c_str()  
                       , strerror( errno ) );

        state = tsReadError;
    }

    return false;
}

}
