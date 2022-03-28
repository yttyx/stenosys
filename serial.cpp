#include <errno.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "device.h"
#include "keyboard_raw.h"
#include "log.h"
#include "misc.h"
#include "serial.h"

#define LOG_SOURCE "SERIAL"

using namespace   stenosys;


namespace stenosys
{

extern C_log log;

C_serial::C_serial()
{
    handle_ = -1;
}

C_serial::~C_serial()
{
    if ( handle_ >= 0 )
    {    
        close( handle_ );
        log_writeln( C_log::LL_INFO, LOG_SOURCE, "Closed serial device" );
    }
}

bool
C_serial::initialise( const std::string & device )
{
    handle_ = open( device.c_str(), O_RDWR | O_NOCTTY | O_SYNC );
    
    if ( handle_ >= 0 )
    {
        // Baud rate 9600, 8 bits, no parity, 1 stop bit
        set_interface_attributes( handle_, B9600 );
        return true;
    }
    else
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "**Error opening output serial device %s: %s"
                                                    , STENO_INPUT_DEVICE
                                                    , strerror( errno ) );
    }

    return false;
}

void
C_serial::stop()
{
    __u16 command = ( EV_KEY_RELEASE_ALL << 8 ) + EV_KEY_NOOP;

    send( command );

    // Wait enough time for the two bytes to be sent (2mS at 9600bps)
    delay( 5 );
}

// This method is used only for sending steno output strings
void
C_serial::send( std::string & str )
{
    for ( unsigned int ii = 0; ii < str.size(); ii++ )
    {
        send( ( __u16 ) ( ( EV_KEY_DOWN << 8 ) + str[ ii ] ) );
        send( ( __u16 ) ( ( EV_KEY_UP   << 8 ) + str[ ii ] ) );
    }
}

bool
C_serial::send( __u16 key_code )
{
    bool worked = true;
    
    //log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "key_code: %04x", key_code );

    {
        __u8 ch = ( key_code >> 8 );

        worked = worked && ( write( handle_, &ch, 1 ) == 1 );
    }
    {
        __u8 ch = ( key_code & 0xff );

        worked = worked && ( write( handle_, &ch, 1 ) == 1 );
    }

    return worked;
}

bool
C_serial::set_interface_attributes( int handle, int baudrate )
{
    struct termios tty;

    if ( tcgetattr( handle, &tty) < 0 )
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "Error from tcgetattr: %s", strerror( errno ) );
        return false;
    }

    cfsetospeed( &tty, ( speed_t ) baudrate );
    cfsetispeed( &tty, ( speed_t ) baudrate );

    tty.c_cflag |= ( CLOCAL | CREAD );           // Ignore modem controls
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;                          // 8-bit characters
    tty.c_cflag &= ~PARENB;                      // No parity bit
    tty.c_cflag &= ~CSTOPB;                      // Only need 1 stop bit
    tty.c_cflag &= ~CRTSCTS;                     // No hardware flow control

    // Setup for non-canonical mode
    tty.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON );
    tty.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
    tty.c_oflag &= ~OPOST;

    // Fetch bytes as they become available
    tty.c_cc[ VMIN ] = 1;
    tty.c_cc[ VTIME ] = 1;

    if ( tcsetattr( handle, TCSANOW, &tty ) != 0 )
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "Error from tcsetattr: %s", strerror( errno ) );
        return false;
    }
    
    return true;
}

}

