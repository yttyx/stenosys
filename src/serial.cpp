#include <errno.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "log.h"
#include "miscellaneous.h"
#include "serial.h"

#define LOG_SOURCE "SERIAL"

using namespace stenosys;


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
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Closed device %s", device_.c_str() );
    }
}

bool
C_serial::initialise( const std::string & device )
{
    device_ = device;
    handle_ = open( device.c_str(), O_RDWR | O_NOCTTY | O_SYNC );
    
    if ( handle_ >= 0 )
    {
        // Baud rate 9600, 8 bits, no parity, 1 stop bit
        set_interface_attributes( handle_, B9600 );
        return true;
    }
    else
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "**Error opening device %s: %s", device_.c_str(), strerror( errno ) );
    }

    return false;
}

bool
C_serial::send( uint8_t ch )
{
    return ::write( handle_, &ch, 1 ) == 1;
}

bool
C_serial::set_interface_attributes( int handle, int baudrate )
{
    struct termios tty;

    if ( tcgetattr( handle, &tty) < 0 )
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "Device %s tcgetattr error: %s", device_.c_str(), strerror( errno ) );
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
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "Device %s tcsetattr error: %s", device_.c_str(), strerror( errno ) );
        return false;
    }
    
    return true;
}

}

