// keyboard.h
// 'Built-in' keyboard (as distinct from the external steno keyboard)

#pragma once

#include <termios.h>

namespace stenosys
{

class C_keyboard
{

public:

    C_keyboard();
    ~C_keyboard();

    bool
    abort();

    void
    wait_for_key();

private:

    bool
    got_keypress();

    int
    get_key();

    static const int ABORT_KEY_1    = 0x03;   // Ctrl-C
    static const int ABORT_KEY_2    = 0x1b;   // <Esc>
    static const int NO_KEY         = -1;

private:

    struct termios termios_original_;
    struct termios termios_current_;

    int kbd_ch_;
};

}
