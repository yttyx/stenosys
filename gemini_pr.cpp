// gemini_pr.cpp

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "device.h"
#include "gemini_pr.h"
#include "log.h"
#include "miscellaneous.h"

#define LOG_SOURCE "GEMIN"

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_gemini_pr::C_gemini_pr()
{
    handle_ = -1;
    abort_  = false;
}

C_gemini_pr::~C_gemini_pr()
{
    if ( handle_ >= 0 )
    {
        close( handle_ );
        log_writeln( C_log::LL_INFO, LOG_SOURCE, "Closed steno device" );
    }
}

// -----------------------------------------------------------------------------------
// Foreground thread code
// -----------------------------------------------------------------------------------
bool
C_gemini_pr::initialise( const std::string & device )
{
    bool worked = false;
    
    handle_ = ::open( device.c_str(), O_RDONLY | O_NONBLOCK );

    if ( handle_ < 0 )
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "**Error opening steno device %s: %s - use sudo?", STENO_INPUT_DEVICE, strerror( errno ) );
    }
    else
    {
        if ( set_interface_attribs( handle_, B19200 ) > -1 )
        {
            log_writeln( C_log::LL_INFO, LOG_SOURCE, "Steno device opened" );
            worked = true;
        }
    }

    return worked;
}

// See https://stackoverflow.com/questions/20154157/termios-vmin-vtime-and-blocking-non-blocking-read-operations

int
C_gemini_pr::set_interface_attribs( int fd, int speed )
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

bool
C_gemini_pr::start()
{
    return thread_start();
}

void
C_gemini_pr::stop()
{
    abort_ = true;

    thread_await_exit();
}

bool
C_gemini_pr::read( std::string & str  )
{
    bool got_data = false;

    buffer_lock_.lock();

    if ( buffer_.size() > 0 )
    {
        str = buffer_.front();
        buffer_.pop();
        got_data = true;
    }

    buffer_lock_.unlock();

    return got_data;
}

enum ePacketState
{
    psAwaitingPacketHeader
,   psPacketBody
};

// -----------------------------------------------------------------------------------
// Background thread code
// -----------------------------------------------------------------------------------

void
C_gemini_pr::thread_handler()
{
    unsigned char packet[ BYTES_PER_STROKE ];
    unsigned int  packet_count = 0;
    unsigned char b = 0;

    ePacketState  packet_state = psAwaitingPacketHeader;

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
                        packet[ packet_count++ ] = b;
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
                        packet[ packet_count++ ] = b;
                        
                        if ( packet_count == BYTES_PER_STROKE )
                        {
                            // We have a complete packet; convert to string version of stroke
                            std::string str = convert_stroke( packet );
        
                            buffer_lock_.lock();
                            buffer_.push( str );
                            buffer_lock_.unlock();

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

bool
C_gemini_pr::get_byte( unsigned char & ch )
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

std::string
C_gemini_pr::convert_stroke( const unsigned char packet[ BYTES_PER_STROKE ] )
{
    //log_writeln_fmt( C_log::LL_VERBOSE_3, LOG_SOURCE, "Stroke (binary): %02x %02x %02x %02x %02x %02x",
    //                                      packet[ 0 ], packet[ 1 ], packet[ 2 ], packet[ 3 ], packet[ 4 ], packet[ 5 ] );

    // Build up keystrokes from the bits set in the received data
    std::string stroke_lhs;
    std::string stroke_rhs;

    for ( unsigned int byte = 0; byte < BYTES_PER_STROKE; byte++ )
    {
        unsigned char b = packet[ byte ];
        
        for ( unsigned int bit = 1; bit <= 7; bit++ )
        {
            if ( ( b << bit ) & 0x80 )
            {
                const char key = steno_key_chart[ ( byte * 7 ) + bit - 1 ];

                //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "key: %c", key );

                if ( byte <= 2 )
                {
                    // LHS 'S' and '*' keys are effectively one ganged key, so suppress a second instance
                    if ( ( key == 'S' ) || ( key == '*' ) )
                    {
                        add_if_unique( key, stroke_lhs );
                    }
                    else
                    {
                        stroke_lhs += key;
                    }
                }
                else
                {
                    // RHS keys
                    stroke_rhs += key;
                }
            }
        }
    }

    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "stroke_lhs: %s,  stroke_rhs: %s"
    //                                           , stroke_lhs.c_str()
    //                                           , stroke_rhs.c_str() );

    if ( ! suppress_hyphen( stroke_lhs, stroke_rhs ) )
    {
        stroke_lhs += "-";
    }

    return stroke_lhs + stroke_rhs;
}

void
C_gemini_pr::add_if_unique( char key, std::string & stroke )
{
    if ( stroke.find( key ) == std::string::npos )
    {
        stroke += key;
    }
}

// "...and if they are not unique then IMPLICIT_HYPHEN_KEYS should be set to the letters that repeat?
//  Benoit Pierre: no
//  Benoit Pierre: the center group of unique letters
//  Benoit Pierre: if one of them is present, there's no need for the hyphen"
// nsjw: IMPLICIT_HYPHEN_KEYS is defined in Plover
//       Review the logic below to see if an improvement can be made

bool
C_gemini_pr::suppress_hyphen( const std::string & lhs, const std::string & rhs )
{
    if ( lhs.find_first_of( "AO*", 0 ) != std::string::npos )
    {
        return true;
    }

    if ( rhs.find_first_of( "EU", 0 ) != std::string::npos )
    {
        return true;
    }

    if ( rhs.length() == 0 )
    {
        return true;
    }

    return false;
}

// When running the Steno layer in QMK, all '*' keys come through as left-hand keys
const char C_gemini_pr::steno_key_chart[] =
{
    '?', '#', '#', '#', '#', '#', '#'   // Left hand keys
,   'S', 'S', 'T', 'K', 'P', 'W', 'H'
,   'R', 'A', 'O', '*', '*', '?', '?'
,   '?', '*', '*', 'E', 'U', 'F', 'R'   // Right hand keys
,   'P', 'B', 'L', 'G', 'T', 'S', 'D'
,   '#', '#', '#', '#', '#', '#', 'Z'
};

}
