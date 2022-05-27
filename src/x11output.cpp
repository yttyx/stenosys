// x11output.cpp
//

#include <X11/X.h>
#include <algorithm>
#include <assert.h>

#include <cstdint>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <X11/Xlib.h>

#include "keyevent.h"
#include "log.h"
#include "miscellaneous.h"
#include "shaviankeysymdefs.h"
#include "utf8.h"
#include "x11output.h"

#define LOG_SOURCE "X11OP"

using namespace stenosys;

namespace stenosys
{

extern C_log log;


C_x11_output::C_x11_output()
    : shavian_( false )
    , display_( nullptr )
{
}

C_x11_output::~C_x11_output()
{
    if ( display_ != NULL )
    {
        XCloseDisplay( display_ );
    }
}

bool
C_x11_output::initialise()
{
    display_ = XOpenDisplay( NULL );

    if ( display_ != NULL )
    {
        set_up_data();
        find_unused_keycodes();
    }

    return display_ != NULL;
}

void
C_x11_output::toggle_shavian()
{
    shavian_ = ! shavian_;
}

void
C_x11_output::send( const std::string & str )
{
    C_utf8 utf8_str( str );

    uint32_t code = 0;

    if ( utf8_str.get_first( code ) )
    {
        do
        {
            if ( ( int ) code <= 0x7f )
            {
                keysym_entry * entry = &ascii_to_keysym[ ( ( int ) code ) ];

                if ( entry->base != 0 )
                {
                    send_key( entry->base, entry->modifier );
                }
            }
            else
            {
                // From keysymdef.h:
                // "For any future extension of the keysyms with characters already
                //  found in ISO 10646 / Unicode, the following algorithm shall be
                //  used. The new keysym code position will simply be the character's
                //  Unicode number plus 0x01000000. The keysym values in the range
                //  0x01000100 to 0x0110ffff are reserved to represent Unicode"
                //
                // 0x10450 is the base value of the Shavian code block
                code = to_keysym( code );

                send_key( code, 0 );
            }

        } while ( utf8_str.get_next( code ) );
    }
}

void
C_x11_output::send( key_event_t key_event, uint8_t scancode )
{
    if ( scancode <= 0x7f )
    {
        // Convert scancode to ASCII
        KeySym keysym = scancode_to_keysym[ scancode ];

        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "scancode: %02xh, keysym: %04xh", scancode, keysym );

        if ( keysym != 0 )
        {
            KeyCode keycode = XKeysymToKeycode( display_, keysym );
         
            if ( keycode != 0 )
            {
                XTestGrabControl( display_, True );

                // Generate regular key press and release
                if ( key_event == KEY_EV_DOWN )
                {
                    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "key down, keycode: %d", keycode );
                    XTestFakeKeyEvent( display_, keycode, True, 0 );
                }
                else if ( key_event == KEY_EV_UP )
                {
                    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "key up, keycode: %d", keycode );
                    XTestFakeKeyEvent( display_, keycode, False, 0 ); 
                } 
             
                XSync( display_, False );
                XTestGrabControl( display_, False );
            }
        }
    }
}

void
C_x11_output::send_key( KeySym keysym, KeySym modsym )
{
    KeyCode keycode = 0;
    KeyCode modcode = 0;

    keycode = XKeysymToKeycode( display_, keysym );
 
    if ( keycode == 0 )
    {
        return;
    }

    XTestGrabControl( display_, True );

    // Generate modkey press
    if ( modsym != 0 )
    {
        modcode = XKeysymToKeycode( display_, modsym );
        XTestFakeKeyEvent( display_, modcode, True, 0 );
    }
 
    // Generate regular key press and release
    XTestFakeKeyEvent( display_, keycode, True, 0 );
    XTestFakeKeyEvent( display_, keycode, False, 0 ); 
 
    // Generate modkey release
    if ( modsym != 0 )
    {
        XTestFakeKeyEvent( display_, modcode, False, 0 );
    }
 
    XSync( display_, False );
    XTestGrabControl( display_, False );
}

void
C_x11_output::send_key( KeyCode keycode )
{
    if ( keycode == 0 )
    {
        return;
    }

    XTestGrabControl( display_, True );

    // Generate regular key press and release
    XTestFakeKeyEvent( display_, keycode, True, 0 );
    XTestFakeKeyEvent( display_, keycode, False, 0 ); 
 
    XSync( display_, False );
    XTestGrabControl( display_, False );
}

void
C_x11_output::set_up_data()
{
    for ( const char ** entry = XF86_symstrings; *entry; entry++ )
    {
        symstrings_.push_back( std::string( *entry ) );
    }
}

void
C_x11_output::find_unused_keycodes()
{
    // Check whether a Shavian KeySym has already been set up
    int keycode = XKeysymToKeycode( display_, to_keysym( XK_peep ) );
 
    if ( keycode != 0 )
    {
        log_writeln( C_log::LL_INFO, LOG_SOURCE, "Shavian codes already set" );
        return;
    }

    int keysyms_per_keycode = 0;
    int keycode_low         = 0;
    int keycode_high        = 0;
    
    KeySym * keysyms = NULL;
    
    // Get the range of keycodes (it's usually the range 8 - 255)
    XDisplayKeycodes( display_, &keycode_low, &keycode_high );

    // Get all of the available mapped keysyms
    keysyms = XGetKeyboardMapping( display_, keycode_low, keycode_high - keycode_low, &keysyms_per_keycode);

    uint32_t shavian = XK_peep;

    // Loop through the keycodes and look for keysyms associated with each keycode
    // that can be set to use Shavian keysyms instead.
    // XF86_symstrings contains a list of keysym strings which seem like good candidates
    // for re-use (they will become unavailable for their original purpose).

    bool done = false;

    for ( int keycode = keycode_low; ( keycode <= keycode_high ) && ( ! done ); keycode++)
    {
        for ( int sym_count = 0; sym_count < keysyms_per_keycode; sym_count++ )
        {
            int sym_index = ( keycode - keycode_low) * keysyms_per_keycode + sym_count;

            KeySym keysym= keysyms[ sym_index ];

            const char * keysymstring = XKeysymToString( keysym ); 

            if ( sym_count == 0 )
            {
                if ( keysymstring != nullptr )
                {
                    std::string entry( keysymstring );

                    if ( std::find( symstrings_.begin(), symstrings_.end(), entry ) != symstrings_.end() )
                    {
                        // Found an entry we can repurpose for Shavian
                        std::string shavian_xstring = format_string("U%05x", shavian );

                        KeySym shavian_sym   = XStringToKeysym( shavian_xstring.c_str() );

                        //TODO Dynamically set size of keysym_list using keysyms_per_keycode
                        KeySym keysym_list[] = { shavian_sym
                                               , shavian_sym
                                               , shavian_sym
                                               , shavian_sym
                                               , shavian_sym
                                               , shavian_sym
                                               , shavian_sym };

                        //TODO Save original keymap so we can restore it on program exit
                        XChangeKeyboardMapping( display_, keycode, keysyms_per_keycode, keysym_list, 1 );

                        if ( shavian == XK_namingdot )
                        {
                            // We're done
                            done = true;
                        }
                        else {

                            shavian++;

                            if ( shavian > XK_yew )
                            {
                                // Shavian alphabet is done, just need to set the middle dot
                                // in the next available slot.
                                shavian = XK_namingdot;
                            }
                        }

                        break;
                    }
                }
                // TODO? We don't currentry make use of keycodes that have no associated KeySyms,
                //       but we could
                // if ( keysymstring == nullptr ) ...
            }
        }
    }
    
    XFree( keysyms );
    XFlush( display_ );
    
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "Shavian codes set" );
}

KeySym
C_x11_output::scancode_to_keysym[] =
{
    0                           // KEY_RESERVED              0
,   XK_Escape                   // KEY_ESC                   1
,   XK_1                        // KEY_1                     2
,   XK_2                        // KEY_2                     3
,   XK_3                        // KEY_3                     4
,   XK_4                        // KEY_4                     5
,   XK_5                        // KEY_5                     6
,   XK_6                        // KEY_6                     7
,   XK_7                        // KEY_7                     8
,   XK_8                        // KEY_8                     9
,   XK_9                        // KEY_9                     10
,   XK_0                        // KEY_0                     11
,   XK_minus                    // KEY_MINUS                 12
,   XK_equal                    // KEY_EQUAL                 13
,   XK_BackSpace                // KEY_BACKSPACE             14
,   XK_Tab                      // KEY_TAB                   15
,   XK_Q                        // KEY_Q                     16
,   XK_W                        // KEY_W                     17
,   XK_E                        // KEY_E                     18
,   XK_R                        // KEY_R                     19
,   XK_T                        // KEY_T                     20
,   XK_Y                        // KEY_Y                     21
,   XK_U                        // KEY_U                     22
,   XK_I                        // KEY_I                     23
,   XK_O                        // KEY_O                     24
,   XK_P                        // KEY_P                     25
,   XK_bracketleft              // KEY_LEFTBRACE             26
,   XK_bracketright             // KEY_RIGHTBRACE            27
,   XK_Return                   // KEY_ENTER                 28
,   XK_Control_L                // KEY_LEFTCTRL              29
,   XK_A                        // KEY_A                     30
,   XK_S                        // KEY_S                     31
,   XK_D                        // KEY_D                     32
,   XK_F                        // KEY_F                     33
,   XK_G                        // KEY_G                     34
,   XK_H                        // KEY_H                     35
,   XK_J                        // KEY_J                     36
,   XK_K                        // KEY_K                     37
,   XK_L                        // KEY_L                     38
,   XK_semicolon                // KEY_SEMICOLON             39
,   XK_apostrophe               // KEY_APOSTROPHE            40
,   XK_grave                    // KEY_GRAVE                 41
,   XK_Shift_L                  // KEY_LEFTSHIFT             42
,   XK_backslash                // KEY_BACKSLASH             43
,   XK_Z                        // KEY_Z                     44
,   XK_X                        // KEY_X                     45
,   XK_C                        // KEY_C                     46
,   XK_V                        // KEY_V                     47
,   XK_B                        // KEY_B                     48
,   XK_N                        // KEY_N                     49
,   XK_M                        // KEY_M                     50
,   XK_comma                    // KEY_COMMA                 51
,   XK_period                   // KEY_DOT                   52
,   XK_slash                    // KEY_SLASH                 53
,   XK_Shift_R                  // KEY_RIGHTSHIFT            54
,   XK_asterisk                 // KEY_KPASTERISK            55
,   XK_Alt_L                    // KEY_LEFTALT               56
,   XK_space                    // KEY_SPACE                 57
,   XK_Caps_Lock                // KEY_CAPSLOCK              58
,   XK_F1                       // KEY_F1                    59
,   XK_F2                       // KEY_F2                    60
,   XK_F3                       // KEY_F3                    61
,   XK_F4                       // KEY_F4                    62
,   XK_F5                       // KEY_F5                    63
,   XK_F6                       // KEY_F6                    64
,   XK_F7                       // KEY_F7                    65
,   XK_F8                       // KEY_F8                    66
,   XK_F9                       // KEY_F9                    67
,   XK_F10                      // KEY_F10                   68
,   XK_Num_Lock                 // KEY_NUMLOCK               69
,   XK_Scroll_Lock              // KEY_SCROLLOCK             70
,   XK_7                        // KEY_KP7                   71
,   XK_8                        // KEY_KP8                   72
,   XK_9                        // KEY_KP9                   73
,   XK_minus                    // KEY_KPMINUS               74
,   XK_4                        // KEY_KP4                   75
,   XK_5                        // KEY_KP5                   76
,   XK_6                        // KEY_KP6                   77
,   XK_plus                     // KEY_KPPLUS                78
,   XK_1                        // KEY_KP1                   79
,   XK_2                        // KEY_KP2                   80
,   XK_3                        // KEY_KP3                   81
,   XK_0                        // KEY_KP0                   82
,   XK_period                   // KEY_KPDOT                 83
,   0                           //                           84
,   0                           // KEY_ZENKAKUHANKAKU        85
,   XK_backslash                // KEY_102ND                 86
,   XK_F11                      // KEY_F11                   87
,   XK_F12                      // KEY_F12                   88
,   0                           // KEY_RO                    89
,   0                           // KEY_KATAKANA              90
,   0                           // KEY_HIRAGANA              91
,   0                           // KEY_HENKAN                92
,   0                           // KEY_KATAKANAHIRAGANA      93
,   0                           // KEY_MUHENKAN              94
,   0                           // KEY_KPJPCOMMA             95
,   0                           // KEY_KPENTER               96
,   XK_Control_R                // KEY_RIGHTCTRL             97
,   0                           // KEY_KPSLASH               98
,   0                           // KEY_SYSRQ                 99
,   XK_Alt_R                    // KEY_RIGHTALT              100
,   0                           // KEY_LINEFEED              101
,   XK_Home                     // KEY_HOME                  102
,   XK_Up                       // KEY_UP                    103
,   XK_Page_Up                  // KEY_PAGEUP                104
,   XK_Left                     // KEY_LEFT                  105
,   XK_Right                    // KEY_RIGHT                 106
,   XK_End                      // KEY_END                   107
,   XK_Down                     // KEY_DOWN                  108
,   XK_Page_Down                // KEY_PAGEDOWN              109
,   XK_Insert                   // KEY_INSERT                110
,   XK_Delete                   // KEY_DELETE                111
,   0                           // KEY_MACRO                 112
,   0                           // KEY_MUTE                  113
,   0                           // KEY_VOLUMEDOWN            114
,   0                           // KEY_VOLUMEUP              115
,   0                           // KEY_POWER                 116
,   0                           // KEY_KPEQUAL               117
,   0                           // KEY_KPPLUSMINUS           118
,   0                           // KEY_PAUSE                 119
,   0                           // KEY_SCALE                 120
,   0                           // KEY_KPCOMMA               121
,   0                           // KEY_HANGEUL               122
,   0                           // KEY_HANJA                 123
,   0                           // KEY_YEN                   124
,   XK_Meta_L                   // KEY_LEFTMETA              125
,   XK_Meta_R                   // KEY_RIGHTMETA             126
,   0                           // KEY_COMPOSE               127
};

keysym_entry
C_x11_output::ascii_to_keysym[] =
{
    { 0,               0          }     // 0000
,   { 0,               0          }     // 0001
,   { 0,               0          }     // 0002
,   { 0,               0          }     // 0003
,   { 0,               0          }     // 0004
,   { 0,               0          }     // 0005
,   { 0,               0          }     // 0006
,   { 0,               0          }     // 0007
,   { XK_BackSpace,    0          }     // 0008
,   { XK_Tab,          0          }     // 0009
,   { XK_Linefeed,     0          }     // 000a
,   { XK_Clear,        0          }     // 000b
,   { 0,               0          }     // 000c
,   { XK_Return,       0          }     // 000d
,   { 0,               0          }     // 000e
,   { 0,               0          }     // 000f
,   { 0,               0          }     // 0010
,   { 0,               0          }     // 0011
,   { 0,               0          }     // 0012
,   { XK_Pause,        0          }     // 0013
,   { XK_Scroll_Lock,  0          }     // 0014
,   { XK_Sys_Req,      0          }     // 0015
,   { 0,               0          }     // 0016
,   { 0,               0          }     // 0017
,   { 0,               0          }     // 0018
,   { 0,               0          }     // 0019
,   { 0,               0          }     // 001a
,   { XK_Escape,       0          }     // 001b
,   { 0,               0          }     // 00lc
,   { 0,               0          }     // 00ld
,   { 0,               0          }     // 00le
,   { 0,               0          }     // 00lf
,   { XK_space,        0          }     // 0020  /* U+0020 SPACE */
,   { XK_exclam,       XK_Shift_L }     // 0021  /* U+0021 EXCLAMATION MARK */
,   { XK_quotedbl,     XK_Shift_L }     // 0022  /* U+0022 QUOTATION MARK */
,   { XK_numbersign,   0          }     // 0023  /* U+0023 NUMBER SIGN */
,   { XK_dollar,       XK_Shift_L }     // 0024  /* U+0024 DOLLAR SIGN */
,   { XK_percent,      XK_Shift_L }     // 0025  /* U+0025 PERCENT SIGN */
,   { XK_ampersand,    XK_Shift_L }     // 0026  /* U+0026 AMPERSAND */
,   { XK_apostrophe,   0          }     // 0027  /* U+0027 APOSTROPHE */
,   { XK_parenleft,    XK_Shift_L }     // 0028  /* U+0028 LEFT PARENTHESIS */
,   { XK_parenright,   XK_Shift_L }     // 0029  /* U+0029 RIGHT PARENTHESIS */
,   { XK_asterisk,     XK_Shift_L }     // 002a  /* U+002A ASTERISK */
,   { XK_plus,         XK_Shift_L }     // 002b  /* U+002B PLUS SIGN */
,   { XK_comma,        0          }     // 002c  /* U+002C COMMA */
,   { XK_minus,        0          }     // 002d  /* U+002D HYPHEN-MINUS */
,   { XK_period,       0          }     // 002e  /* U+002E FULL STOP */
,   { XK_slash,        0          }     // 002f  /* U+002F SOLIDUS */
,   { XK_0,            0          }     // 0030  /* U+0030 DIGIT ZERO */
,   { XK_1,            0          }     // 0031  /* U+0031 DIGIT ONE */
,   { XK_2,            0          }     // 0032  /* U+0032 DIGIT TWO */
,   { XK_3,            0          }     // 0033  /* U+0033 DIGIT THREE */
,   { XK_4,            0          }     // 0034  /* U+0034 DIGIT FOUR */
,   { XK_5,            0          }     // 0035  /* U+0035 DIGIT FIVE */
,   { XK_6,            0          }     // 0036  /* U+0036 DIGIT SIX */
,   { XK_7,            0          }     // 0037  /* U+0037 DIGIT SEVEN */
,   { XK_8,            0          }     // 0038  /* U+0038 DIGIT EIGHT */
,   { XK_9,            0          }     // 0039  /* U+0039 DIGIT NINE */
,   { XK_colon,        XK_Shift_L }     // 003a  /* U+003A COLON */
,   { XK_semicolon,    0          }     // 003b  /* U+003B SEMICOLON */
,   { XK_less,         XK_Shift_L }     // 003c  /* U+003C LESS-THAN SIGN */
,   { XK_equal,        0          }     // 003d  /* U+003D EQUALS SIGN */
,   { XK_greater,      XK_Shift_L }     // 003e  /* U+003E GREATER-THAN SIGN */
,   { XK_question,     XK_Shift_L }     // 003f  /* U+003F QUESTION MARK */
,   { XK_at,           0          }     // 0040  /* U+0040 COMMERCIAL AT */
,   { XK_A,            XK_Shift_L }     // 0041  /* U+0041 LATIN CAPITAL LETTER A */
,   { XK_B,            XK_Shift_L }     // 0042  /* U+0042 LATIN CAPITAL LETTER B */
,   { XK_C,            XK_Shift_L }     // 0043  /* U+0043 LATIN CAPITAL LETTER C */
,   { XK_D,            XK_Shift_L }     // 0044  /* U+0044 LATIN CAPITAL LETTER D */
,   { XK_E,            XK_Shift_L }     // 0045  /* U+0045 LATIN CAPITAL LETTER E */
,   { XK_F,            XK_Shift_L }     // 0046  /* U+0046 LATIN CAPITAL LETTER F */
,   { XK_G,            XK_Shift_L }     // 0047  /* U+0047 LATIN CAPITAL LETTER G */
,   { XK_H,            XK_Shift_L }     // 0048  /* U+0048 LATIN CAPITAL LETTER H */
,   { XK_I,            XK_Shift_L }     // 0049  /* U+0049 LATIN CAPITAL LETTER I */
,   { XK_J,            XK_Shift_L }     // 004a  /* U+004A LATIN CAPITAL LETTER J */
,   { XK_K,            XK_Shift_L }     // 004b  /* U+004B LATIN CAPITAL LETTER K */
,   { XK_L,            XK_Shift_L }     // 004c  /* U+004C LATIN CAPITAL LETTER L */
,   { XK_M,            XK_Shift_L }     // 004d  /* U+004D LATIN CAPITAL LETTER M */
,   { XK_N,            XK_Shift_L }     // 004e  /* U+004E LATIN CAPITAL LETTER N */
,   { XK_O,            XK_Shift_L }     // 004f  /* U+004F LATIN CAPITAL LETTER O */
,   { XK_P,            XK_Shift_L }     // 0050  /* U+0050 LATIN CAPITAL LETTER P */
,   { XK_Q,            XK_Shift_L }     // 0051  /* U+0051 LATIN CAPITAL LETTER Q */
,   { XK_R,            XK_Shift_L }     // 0052  /* U+0052 LATIN CAPITAL LETTER R */
,   { XK_S,            XK_Shift_L }     // 0053  /* U+0053 LATIN CAPITAL LETTER S */
,   { XK_T,            XK_Shift_L }     // 0054  /* U+0054 LATIN CAPITAL LETTER T */
,   { XK_U,            XK_Shift_L }     // 0055  /* U+0055 LATIN CAPITAL LETTER U */
,   { XK_V,            XK_Shift_L }     // 0056  /* U+0056 LATIN CAPITAL LETTER V */
,   { XK_W,            XK_Shift_L }     // 0057  /* U+0057 LATIN CAPITAL LETTER W */
,   { XK_X,            XK_Shift_L }     // 0058  /* U+0058 LATIN CAPITAL LETTER X */
,   { XK_Y,            XK_Shift_L }     // 0059  /* U+0059 LATIN CAPITAL LETTER Y */
,   { XK_Z,            XK_Shift_L }     // 005a  /* U+005A LATIN CAPITAL LETTER Z */
,   { XK_bracketleft,  0          }     // 005b  /* U+005B LEFT SQUARE BRACKET */
,   { XK_backslash,    0          }     // 005c  /* U+005C REVERSE SOLIDUS */
,   { XK_bracketright, 0          }     // 005d  /* U+005D RIGHT SQUARE BRACKET */
,   { XK_asciicircum,  XK_Shift_L }     // 005e  /* U+005E CIRCUMFLEX ACCENT */
,   { XK_underscore,   XK_Shift_L }     // 005f  /* U+005F LOW LINE */
,   { XK_grave,        0          }     // 0060  /* U+0060 GRAVE ACCENT */
,   { XK_a,            0          }     // 0061  /* U+0061 LATIN SMALL LETTER A */
,   { XK_b,            0          }     // 0062  /* U+0062 LATIN SMALL LETTER B */
,   { XK_c,            0          }     // 0063  /* U+0063 LATIN SMALL LETTER C */
,   { XK_d,            0          }     // 0064  /* U+0064 LATIN SMALL LETTER D */
,   { XK_e,            0          }     // 0065  /* U+0065 LATIN SMALL LETTER E */
,   { XK_f,            0          }     // 0066  /* U+0066 LATIN SMALL LETTER F */
,   { XK_g,            0          }     // 0067  /* U+0067 LATIN SMALL LETTER G */
,   { XK_h,            0          }     // 0068  /* U+0068 LATIN SMALL LETTER H */
,   { XK_i,            0          }     // 0069  /* U+0069 LATIN SMALL LETTER I */
,   { XK_j,            0          }     // 006a  /* U+006A LATIN SMALL LETTER J */
,   { XK_k,            0          }     // 006b  /* U+006B LATIN SMALL LETTER K */
,   { XK_l,            0          }     // 006c  /* U+006C LATIN SMALL LETTER L */
,   { XK_m,            0          }     // 006d  /* U+006D LATIN SMALL LETTER M */
,   { XK_n,            0          }     // 006e  /* U+006E LATIN SMALL LETTER N */
,   { XK_o,            0          }     // 006f  /* U+006F LATIN SMALL LETTER O */
,   { XK_p,            0          }     // 0070  /* U+0070 LATIN SMALL LETTER P */
,   { XK_q,            0          }     // 0071  /* U+0071 LATIN SMALL LETTER Q */
,   { XK_r,            0          }     // 0072  /* U+0072 LATIN SMALL LETTER R */
,   { XK_s,            0          }     // 0073  /* U+0073 LATIN SMALL LETTER S */
,   { XK_t,            0          }     // 0074  /* U+0074 LATIN SMALL LETTER T */
,   { XK_u,            0          }     // 0075  /* U+0075 LATIN SMALL LETTER U */
,   { XK_v,            0          }     // 0076  /* U+0076 LATIN SMALL LETTER V */
,   { XK_w,            0          }     // 0077  /* U+0077 LATIN SMALL LETTER W */
,   { XK_x,            0          }     // 0078  /* U+0078 LATIN SMALL LETTER X */
,   { XK_y,            0          }     // 0079  /* U+0079 LATIN SMALL LETTER Y */
,   { XK_z,            0          }     // 007a  /* U+007A LATIN SMALL LETTER Z */
,   { XK_braceleft,    0          }     // 007b  /* U+007B LEFT CURLY BRACKET */
,   { XK_bar,          XK_Shift_L }     // 007c  /* U+007C VERTICAL LINE */
,   { XK_braceright,   0          }     // 007d  /* U+007D RIGHT CURLY BRACKET */
,   { XK_asciitilde,   XK_Shift_L }     // 007e  /* U+007E TILDE */
};

//WIP
keysym_entry
C_x11_output::ascii_to_shavian_keysym[] =
{
    { XK_A,            XK_Shift_L }     // 0041  /* U+0041 LATIN CAPITAL LETTER A */
,   { XK_B,            XK_Shift_L }     // 0042  /* U+0042 LATIN CAPITAL LETTER B */
,   { XK_C,            XK_Shift_L }     // 0043  /* U+0043 LATIN CAPITAL LETTER C */
,   { XK_D,            XK_Shift_L }     // 0044  /* U+0044 LATIN CAPITAL LETTER D */
,   { XK_E,            XK_Shift_L }     // 0045  /* U+0045 LATIN CAPITAL LETTER E */
,   { XK_F,            XK_Shift_L }     // 0046  /* U+0046 LATIN CAPITAL LETTER F */
,   { XK_G,            XK_Shift_L }     // 0047  /* U+0047 LATIN CAPITAL LETTER G */
,   { XK_H,            XK_Shift_L }     // 0048  /* U+0048 LATIN CAPITAL LETTER H */
,   { XK_I,            XK_Shift_L }     // 0049  /* U+0049 LATIN CAPITAL LETTER I */
,   { XK_J,            XK_Shift_L }     // 004a  /* U+004A LATIN CAPITAL LETTER J */
,   { XK_K,            XK_Shift_L }     // 004b  /* U+004B LATIN CAPITAL LETTER K */
,   { XK_L,            XK_Shift_L }     // 004c  /* U+004C LATIN CAPITAL LETTER L */
,   { XK_M,            XK_Shift_L }     // 004d  /* U+004D LATIN CAPITAL LETTER M */
,   { XK_N,            XK_Shift_L }     // 004e  /* U+004E LATIN CAPITAL LETTER N */
,   { XK_O,            XK_Shift_L }     // 004f  /* U+004F LATIN CAPITAL LETTER O */
,   { XK_P,            XK_Shift_L }     // 0050  /* U+0050 LATIN CAPITAL LETTER P */
,   { XK_Q,            XK_Shift_L }     // 0051  /* U+0051 LATIN CAPITAL LETTER Q */
,   { XK_R,            XK_Shift_L }     // 0052  /* U+0052 LATIN CAPITAL LETTER R */
,   { XK_S,            XK_Shift_L }     // 0053  /* U+0053 LATIN CAPITAL LETTER S */
,   { XK_T,            XK_Shift_L }     // 0054  /* U+0054 LATIN CAPITAL LETTER T */
,   { XK_U,            XK_Shift_L }     // 0055  /* U+0055 LATIN CAPITAL LETTER U */
,   { XK_V,            XK_Shift_L }     // 0056  /* U+0056 LATIN CAPITAL LETTER V */
,   { XK_W,            XK_Shift_L }     // 0057  /* U+0057 LATIN CAPITAL LETTER W */
,   { XK_X,            XK_Shift_L }     // 0058  /* U+0058 LATIN CAPITAL LETTER X */
,   { XK_Y,            XK_Shift_L }     // 0059  /* U+0059 LATIN CAPITAL LETTER Y */
,   { XK_Z,            XK_Shift_L }     // 005a  /* U+005A LATIN CAPITAL LETTER Z */
,   { XK_a,            0          }     // 0061  /* U+0061 LATIN SMALL LETTER A */
,   { XK_b,            0          }     // 0062  /* U+0062 LATIN SMALL LETTER B */
,   { XK_c,            0          }     // 0063  /* U+0063 LATIN SMALL LETTER C */
,   { XK_d,            0          }     // 0064  /* U+0064 LATIN SMALL LETTER D */
,   { XK_e,            0          }     // 0065  /* U+0065 LATIN SMALL LETTER E */
,   { XK_f,            0          }     // 0066  /* U+0066 LATIN SMALL LETTER F */
,   { XK_g,            0          }     // 0067  /* U+0067 LATIN SMALL LETTER G */
,   { XK_h,            0          }     // 0068  /* U+0068 LATIN SMALL LETTER H */
,   { XK_i,            0          }     // 0069  /* U+0069 LATIN SMALL LETTER I */
,   { XK_j,            0          }     // 006a  /* U+006A LATIN SMALL LETTER J */
,   { XK_k,            0          }     // 006b  /* U+006B LATIN SMALL LETTER K */
,   { XK_l,            0          }     // 006c  /* U+006C LATIN SMALL LETTER L */
,   { XK_m,            0          }     // 006d  /* U+006D LATIN SMALL LETTER M */
,   { XK_n,            0          }     // 006e  /* U+006E LATIN SMALL LETTER N */
,   { XK_o,            0          }     // 006f  /* U+006F LATIN SMALL LETTER O */
,   { XK_p,            0          }     // 0070  /* U+0070 LATIN SMALL LETTER P */
,   { XK_q,            0          }     // 0071  /* U+0071 LATIN SMALL LETTER Q */
,   { XK_r,            0          }     // 0072  /* U+0072 LATIN SMALL LETTER R */
,   { XK_s,            0          }     // 0073  /* U+0073 LATIN SMALL LETTER S */
,   { XK_t,            0          }     // 0074  /* U+0074 LATIN SMALL LETTER T */
,   { XK_u,            0          }     // 0075  /* U+0075 LATIN SMALL LETTER U */
,   { XK_v,            0          }     // 0076  /* U+0076 LATIN SMALL LETTER V */
,   { XK_w,            0          }     // 0077  /* U+0077 LATIN SMALL LETTER W */
,   { XK_x,            0          }     // 0078  /* U+0078 LATIN SMALL LETTER X */
,   { XK_y,            0          }     // 0079  /* U+0079 LATIN SMALL LETTER Y */
,   { XK_z,            0          }     // 007a  /* U+007A LATIN SMALL LETTER Z */
};

// Array of symkey strings whose references to keycodes in the keyboard
// will have Shavian code point substituted in their stead.
const char * C_x11_output::XF86_symstrings[] =
{
    "XF86AudioMicMute" 
,   "XF86AudioPause" 
,   "XF86AudioPlay" 
,   "XF86AudioPreset" 
,   "XF86Battery" 
,   "XF86Bluetooth" 
,   "XF86BrightnessAuto" 
,   "XF86DOS" 
,   "XF86DisplayOff" 
,   "XF86Documents" 
,   "XF86Favorites" 
,   "XF86Finance" 
,   "XF86Game" 
,   "XF86Go" 
,   "XF86HomePage" 
,   "XF86HomePage" 
,   "XF86Launch1"
,   "XF86Launch2"
,   "XF86Launch3"
,   "XF86Launch4"
,   "XF86Launch5"
,   "XF86Launch6"
,   "XF86Launch7"
,   "XF86Launch8"
,   "XF86Launch9"
,   "XF86LaunchA"
,   "XF86LaunchB"
,   "XF86Mail" 
,   "XF86Mail" 
,   "XF86MailForward" 
,   "XF86Messenger" 
,   "XF86MonBrightnessCycle" 
,   "XF86MyComputer" 
,   "XF86New" 
,   "XF86Next_VMode" 
,   "XF86Prev_VMode" 
,   "XF86Reply" 
,   "XF86Save" 
,   "XF86ScreenSaver" 
,   "XF86Search" 
,   "XF86Send" 
,   "XF86Shop" 
,   "XF86Tools" 
,   "XF86TouchpadOff" 
,   "XF86TouchpadOn" 
,   "XF86TouchpadToggle" 
,   "XF86WLAN" 
,   "XF86WWW" 
,   "XF86WebCam" 
,   "XF86Xfer" 
,   nullptr
};

#if 0
uint8_t
C_x11_output::scancode_to_ascii[] =
{
    '?'                         // KEY_RESERVED              0
,   '?'                         // KEY_ESC                   1
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
,   '?'                         // KEY_BACKSPACE             14
,   '?'                         // KEY_TAB                   15
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
,   '?'                         // KEY_ENTER                 28
,   '?'                         // KEY_LEFTCTRL              29
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
,   '?'                         // KEY_LEFTSHIFT             42
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
,   '?'                         // KEY_RIGHTSHIFT            54
,   '*'                         // KEY_KPASTERISK            55
,   '?'                         // KEY_LEFTALT               56
,   ' '                         // KEY_SPACE                 57
,   '?'                         // KEY_CAPSLOCK              58
,   '?'                         // KEY_F1                    59
,   '?'                         // KEY_F2                    60
,   '?'                         // KEY_F3                    61
,   '?'                         // KEY_F4                    62
,   '?'                         // KEY_F5                    63
,   '?'                         // KEY_F6                    64
,   '?'                         // KEY_F7                    65
,   '?'                         // KEY_F8                    66
,   '?'                         // KEY_F9                    67
,   '?'                         // KEY_F10                   68
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
,   '?'                         // KEY_F11                   87
,   '?'                         // KEY_F12                   88
,   '?'                         // KEY_RO                    89
,   '?'                         // KEY_KATAKANA              90
,   '?'                         // KEY_HIRAGANA              91
,   '?'                         // KEY_HENKAN                92
,   '?'                         // KEY_KATAKANAHIRAGANA      93
,   '?'                         // KEY_MUHENKAN              94
,   '?'                         // KEY_KPJPCOMMA             95
,   '?'                         // KEY_KPENTER               96
,   '?'                         // KEY_RIGHTCTRL             97
,   '?'                         // KEY_KPSLASH               98
,   '?'                         // KEY_SYSRQ                 99
,   '?'                         // KEY_RIGHTALT              100
,   '?'                         // KEY_LINEFEED              101
,   '?'                         // KEY_HOME                  102
,   '?'                         // KEY_UP                    103
,   '?'                         // KEY_PAGEUP                104
,   '?'                         // KEY_LEFT                  105
,   '?'                         // KEY_RIGHT                 106
,   '?'                         // KEY_END                   107
,   '?'                         // KEY_DOWN                  108
,   '?'                         // KEY_PAGEDOWN              109
,   '?'                         // KEY_INSERT                110
,   '?'                         // KEY_DELETE                111
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
,   '?'                         // KEY_LEFTMETA              125
,   '?'                         // KEY_RIGHTMETA             126
,   '?'                         // KEY_COMPOSE               127
};
#endif

void
C_x11_output::test()
{
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "C_x11_output::test" );

    C_utf8 shav_test ( "·𐑢𐑣𐑩𐑤𐑴 ·𐑢𐑻𐑤𐑛" );

    uint32_t code;

    if ( shav_test.get_first( code ) )
    {
        do
        {
            // From keysymdef.h:
            // "For any future extension of the keysyms with characters already
            //  found in ISO 10646 / Unicode, the following algorithm shall be
            //  used. The new keysym code position will simply be the character's
            //  Unicode number plus 0x01000000. The keysym values in the range
            //  0x01000100 to 0x0110ffff are reserved to represent Unicode"
            // 0x10450 is the base value of the Shavian code block
            if ( code >= 0x10450 )
            {
                code += 0x1000000;
            }

            send_key( code, 0 );

        } while ( shav_test.get_next( code ) );
    }

    send_key( XK_space, 0 );
    send_key( XK_space, 0 );
    send_key( XK_space, 0 );
    
    send_key( XK_quotedbl, XK_Shift_L);
    send_key( XK_H, XK_Shift_L );
    send_key( XK_E, 0 );
    send_key( XK_L, 0 );
    send_key( XK_L, 0 );
    send_key( XK_O, 0 );
    send_key( XK_space, 0 );

    send_key( XK_W, XK_Shift_L );
    send_key( XK_O, 0 );
    send_key( XK_R, 0 );
    send_key( XK_L, 0 );
    send_key( XK_D, 0 );
    //send_key( XK_exclam, XK_Shift_L );    // gives '!'
    send_key( XK_exclam, XK_Shift_L );      // gives '1'
    send_key( XK_quotedbl, XK_Shift_L );
    send_key( XK_Return, 0 );
}

}
