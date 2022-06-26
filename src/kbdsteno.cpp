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
#include <string.h>
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
    buffer_ = std::make_unique< C_buffer< S_geminipr_packet > >();
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
    bool worked = false;
    
    handle_ = ::open( device.c_str(), O_RDONLY | O_NONBLOCK );

    if ( handle_ < 0 )
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "**Error opening steno device %s: %s - use sudo?", device.c_str(), strerror( errno ) );
    }
    else
    {
        if ( set_interface_attributes( handle_, B19200 ) > -1 )
        {
            log_writeln( C_log::LL_VERBOSE_1, LOG_SOURCE, "Steno device opened" );
            worked = true;
        }
    }

    return worked;
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

enum ePacketState
{
    psAwaitingPacketHeader
,   psPacketBody
};

void
C_kbd_steno::thread_handler()
{
    unsigned int  packet_count = 0;
    unsigned char b            = 0;
    ePacketState  packet_state = psAwaitingPacketHeader;

    S_geminipr_packet packet;

    while ( ! abort_ )
    {
        if ( get_byte( b ) )
        {
            switch ( packet_state )
            {
                case psAwaitingPacketHeader:

                    // Packet header has top bit set
                    if ( b & 0x80 )
                    {
                        packet[ packet_count++ ]= b;
                        packet_state = psPacketBody;
                    }
                    break;
                
                case psPacketBody:
                
                    // Rest of packet bytes must have the top bit as zero
                    if ( b & 0x80 )
                    {
                        packet_count = 0;
                        packet_state = psAwaitingPacketHeader;
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
                            packet_state = psAwaitingPacketHeader;
                        }
                    }
                    break;
            }

            if ( packet_count > BYTES_PER_STROKE )
            {
                log_writeln( C_log::LL_ERROR, LOG_SOURCE, "Steno packet too long" );
            }
        }
        else
        {
            delay( 10 );
        }
    }
}
    
// Fetch a block of data in GeminiPR format

bool
C_kbd_steno::get_byte( unsigned char & ch )
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
        // No data available
        return false;
    }
    else
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "**Steno read error %s", strerror( errno ) );
    }

    return false;
}

}
