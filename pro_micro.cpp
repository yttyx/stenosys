// pro_micro.cpp

#include <stdint.h>

#include "miscellaneous.h"
#include "pro_micro.h"

using namespace stenosys;

namespace stenosys
{

bool
C_pro_micro::send( uint16_t key_code )
{
    bool worked = true;
    
    {
        uint8_t ch = ( key_code >> 8 );

        worked = worked && serial_.send( ch );
    }
    {
        uint8_t ch = ( key_code & 0xff );

        worked = worked && serial_.send( ch );
    }

    return worked;
}

// This method is used only for sending steno output strings
void
C_pro_micro::send( std::string & str )
{
    for ( unsigned int ii = 0; ii < str.size(); ii++ )
    {
        send( ( uint16_t ) ( ( EV_KEY_DOWN << 8 ) + str[ ii ] ) );
        send( ( uint16_t ) ( ( EV_KEY_UP   << 8 ) + str[ ii ] ) );
    }
}

void
C_pro_micro::stop()
{
    uint16_t command = ( EV_KEY_RELEASE_ALL << 8 ) + EV_KEY_NOOP;

    send( command );

    // Wait enough time for the two bytes to be sent (2mS at 9600bps)
    delay( 5 );
}

// The first version of this table will convert from the raw keycode to ASCII
// Ultimately the raw keycodes will be sent to the Pro Micro for conversion there
// (this will be done as part of adding support for Plover key commands, when we 
// have to separate out key up and key down events so that key combinations such as
// Ctrl-Alt-S etc can work).

unsigned char
C_pro_micro::keytable[] =
{
    '?'                         // KEY_RESERVED              0
,   ARDUINO_KEY_ESC             // KEY_ESC                   1
,   '1'                         // KEY_1                     2
,   '2'                         // KEY_2                     3
,   '3'                         // KEY_3                     4
,   '4'                         // KEY_4                     5
,   '5'                         // KEY_5                     6
,   '6'                         // KEY_6                     7
,   '7'                         // KEY_7                     8
,   '8'                         // KEY_8                     9
,   '9'                         // KEY_9                     10
,   '0'                         // KEY_0                     11
,   '-'                         // KEY_MINUS                 12
,   '='                         // KEY_EQUAL                 13
,   ARDUINO_KEY_BACKSPACE       // KEY_BACKSPACE             14
,   ARDUINO_KEY_TAB             // KEY_TAB                   15
,   'q'                         // KEY_Q                     16
,   'w'                         // KEY_W                     17
,   'e'                         // KEY_E                     18
,   'r'                         // KEY_R                     19
,   't'                         // KEY_T                     20
,   'y'                         // KEY_Y                     21
,   'u'                         // KEY_U                     22
,   'i'                         // KEY_I                     23
,   'o'                         // KEY_O                     24
,   'p'                         // KEY_P                     25
,   '['                         // KEY_LEFTBRACE             26  yttyx
,   ']'                         // KEY_RIGHTBRACE            27
,   ARDUINO_KEY_RETURN          // KEY_ENTER                 28
,   ARDUINO_KEY_LEFT_CTRL       // KEY_LEFTCTRL              29
,   'a'                         // KEY_A                     30
,   's'                         // KEY_S                     31
,   'd'                         // KEY_D                     32
,   'f'                         // KEY_F                     33
,   'g'                         // KEY_G                     34
,   'h'                         // KEY_H                     35
,   'j'                         // KEY_J                     36
,   'k'                         // KEY_K                     37
,   'l'                         // KEY_L                     38
,   ';'                         // KEY_SEMICOLON             39
,   '\''                        // KEY_APOSTROPHE            40
,   '`'                         // KEY_GRAVE                 41
,   ARDUINO_KEY_LEFT_SHIFT      // KEY_LEFTSHIFT             42
,   '#'                         // KEY_BACKSLASH             43  yttyx
,   'z'                         // KEY_Z                     44
,   'x'                         // KEY_X                     45
,   'c'                         // KEY_C                     46
,   'v'                         // KEY_V                     47
,   'b'                         // KEY_B                     48
,   'n'                         // KEY_N                     49
,   'm'                         // KEY_M                     50
,   ','                         // KEY_COMMA                 51
,   '.'                         // KEY_DOT                   52
,   '/'                         // KEY_SLASH                 53
,   ARDUINO_KEY_RIGHT_SHIFT     // KEY_RIGHTSHIFT            54
,   '*'                         // KEY_KPASTERISK            55
,   ARDUINO_KEY_LEFT_ALT        // KEY_LEFTALT               56
,   ' '                         // KEY_SPACE                 57
,   ARDUINO_KEY_CAPS_LOCK       // KEY_CAPSLOCK              58
,   ARDUINO_KEY_F1              // KEY_F1                    59
,   ARDUINO_KEY_F2              // KEY_F2                    60
,   ARDUINO_KEY_F3              // KEY_F3                    61
,   ARDUINO_KEY_F4              // KEY_F4                    62
,   ARDUINO_KEY_F5              // KEY_F5                    63
,   ARDUINO_KEY_F6              // KEY_F6                    64
,   ARDUINO_KEY_F7              // KEY_F7                    65
,   ARDUINO_KEY_F8              // KEY_F8                    66
,   ARDUINO_KEY_F9              // KEY_F9                    67
,   ARDUINO_KEY_F10             // KEY_F10                   68
,   '?'                         // KEY_NUMLOCK               69
,   '?'                         // KEY_SCROLLLOCK            70
,   '7'                         // KEY_KP7                   71
,   '8'                         // KEY_KP8                   72
,   '9'                         // KEY_KP9                   73
,   '-'                         // KEY_KPMINUS               74
,   '4'                         // KEY_KP4                   75
,   '5'                         // KEY_KP5                   76
,   '6'                         // KEY_KP6                   77
,   '+'                         // KEY_KPPLUS                78
,   '1'                         // KEY_KP1                   79
,   '2'                         // KEY_KP2                   80
,   '3'                         // KEY_KP3                   81
,   '0'                         // KEY_KP0                   82
,   '.'                         // KEY_KPDOT                 83
,   '?'                         //                           84
,   '?'                         // KEY_ZENKAKUHANKAKU        85
,   '\\'                        // KEY_102ND                 86
,   ARDUINO_KEY_F11             // KEY_F11                   87
,   ARDUINO_KEY_F12             // KEY_F12                   88
,   '?'                         // KEY_RO                    89
,   '?'                         // KEY_KATAKANA              90
,   '?'                         // KEY_HIRAGANA              91
,   '?'                         // KEY_HENKAN                92
,   '?'                         // KEY_KATAKANAHIRAGANA      93
,   '?'                         // KEY_MUHENKAN              94
,   '?'                         // KEY_KPJPCOMMA             95
,   '?'                         // KEY_KPENTER               96
,   ARDUINO_KEY_RIGHT_CTRL      // KEY_RIGHTCTRL             97
,   '?'                         // KEY_KPSLASH               98
,   '?'                         // KEY_SYSRQ                 99
,   ARDUINO_KEY_RIGHT_ALT       // KEY_RIGHTALT              100
,   '?'                         // KEY_LINEFEED              101
,   ARDUINO_KEY_HOME            // KEY_HOME                  102
,   ARDUINO_KEY_UP_ARROW        // KEY_UP                    103
,   ARDUINO_KEY_PAGE_UP         // KEY_PAGEUP                104
,   ARDUINO_KEY_LEFT_ARROW      // KEY_LEFT                  105
,   ARDUINO_KEY_RIGHT_ARROW     // KEY_RIGHT                 106
,   ARDUINO_KEY_END             // KEY_END                   107
,   ARDUINO_KEY_DOWN_ARROW      // KEY_DOWN                  108
,   ARDUINO_KEY_PAGE_DOWN       // KEY_PAGEDOWN              109
,   ARDUINO_KEY_INSERT          // KEY_INSERT                110
,   ARDUINO_KEY_DELETE          // KEY_DELETE                111
,   '?'                         // KEY_MACRO                 112
,   '?'                         // KEY_MUTE                  113
,   '?'                         // KEY_VOLUMEDOWN            114
,   '?'                         // KEY_VOLUMEUP              115
,   '?'                         // KEY_POWER                 116
,   '?'                         // KEY_KPEQUAL               117
,   '?'                         // KEY_KPPLUSMINUS           118
,   '?'                         // KEY_PAUSE                 119
,   '?'                         // KEY_SCALE                 120
,   '?'                         // KEY_KPCOMMA               121
,   '?'                         // KEY_HANGEUL               122
,   '?'                         // KEY_HANJA                 123
,   '?'                         // KEY_YEN                   124
,   ARDUINO_KEY_LEFT_GUI        // KEY_LEFTMETA              125
,   ARDUINO_KEY_LEFT_GUI        // KEY_RIGHTMETA             126
,   '?'                         // KEY_COMPOSE               127
};

}
