// C_keyboard.cpp

#include <unistd.h>

#include "keyboard.h"
#include "log.h"

#define LOG_SOURCE "KEYB "

namespace stenosys
{

C_keyboard kbd;
extern C_log log;

C_keyboard::C_keyboard()
{
    tcgetattr( 0, &termios_original_ );

    termios_current_ = termios_original_;

    termios_current_.c_lflag &= ~ICANON;     // select non-canonical mode (don't wait for line terminator)
    termios_current_.c_lflag &= ~ECHO;       // don't echo input characters
    termios_current_.c_lflag &= ~ISIG;       // if INTR, QUIT, SUSP, or DSUSP are received, don't generate the corresponding signal
                                             // (pass through the Ctrl-C, Ctrl-Z etc.)

    //termios_current_.c_cc[ VMIN ]  = 1;  not used in non-canonical mode
    //termios_current_.c_cc[ VTIME ] = 0;

    tcsetattr( 0, TCSANOW, &termios_current_ );

    kbd_ch_ = NO_KEY;
}

C_keyboard::~C_keyboard()
{
    // Restore original setup
    tcsetattr( 0, TCSANOW, &termios_original_ );
}

bool
C_keyboard::abort()
{
    int ch = C_keyboard::NO_KEY;

    if ( got_keypress() )
    {
        log_writeln( C_log::LL_ERROR, LOG_SOURCE, "got keypress" );

        ch = get_key();
    }

    return ( ch == C_keyboard::ABORT_KEY_1 ) || ( ch == C_keyboard::ABORT_KEY_2 );
}

void
C_keyboard::wait_for_key()
{
    while ( ! got_keypress() )
    {
    }

    get_key();
}

bool
C_keyboard::got_keypress()
{
    if ( kbd_ch_ == NO_KEY )
    {
        termios_current_.c_cc[ VMIN ] = 0;

        tcsetattr( 0, TCSANOW, &termios_current_ );

        unsigned char ch;
        int           nread;

        nread = read( 0, &ch, 1 );

        termios_current_.c_cc[ VMIN ] = 1;

        tcsetattr( 0, TCSANOW, &termios_current_ );

        if ( nread == 1 )
        {
            kbd_ch_ = ch;
            return true;
        }
    }

    return false;
}

int
C_keyboard::get_key()
{
    char ch;

    if ( kbd_ch_ != NO_KEY )
    {
        ch = kbd_ch_;
        kbd_ch_ = NO_KEY;
    }
    else
    {
        read( 0, &ch, 1 );
    }

    return ch;
}

}
