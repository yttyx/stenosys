// x11output.cpp

#ifdef X11

#include <algorithm>
#include <assert.h>

#include <cstdint>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include "keyevent.h"
#include "log.h"
#include "miscellaneous.h"
#include "shaviankeysymdefs.h"
#include "utf8.h"
#include "x11output.h"

#define  LOG_SOURCE "X11OP"

using namespace stenosys;

namespace stenosys
{

extern C_log log;


C_x11_output::C_x11_output()
    : shavian_( false )
    , shift_( false )
    , shift_prev_( false )
    , display_( nullptr )
{
    keysym_replacements_= std::make_unique< std::unordered_map< std::string, keysym_entry > >();
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
        set_shavian_keysyms();
    }

    return display_ != NULL;
}

bool
C_x11_output::initialise( const std::string & output_device )
{
    return false;
}

void
C_x11_output::toggle_shavian()
{
    shavian_ = ! shavian_;
}

void
C_x11_output::send( const std::string & str )
{
    //TEMP
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "C_x11_output::send() - str: %s", str.c_str() );

    C_utf8 utf8_str( str );

    uint32_t code = 0;

    if ( utf8_str.get_first( code ) )
    {
        do
        {
            //TEMP
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "code: %04xh", code );
        
            if ( ( int ) code <= 0x7f )
            {
                keysym_entry * entry = &ascii_to_keysym[ ( ( int ) code ) ];

                if ( entry->keysym1 != 0 )
                {
                    send_key( entry->keysym1, entry->keysym2 );
                }
            }
            else
            {
                //TEMP
                log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "is_shavian_code: %s", is_shavian_code( code ) ? "is shavian" : "is NOT shavian" );
                
                if ( is_shavian_code( code ) )
                {
                    if ( ( code == XK_namingdot ) || ( code == XK_acroring ) )
                    {
                        KeySym keysym = to_keysym( code );
        
                        //TEMP
                        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "keysym: %04xh, code: %04xh", keysym, code );
                        
                        send_key( keysym, 0 );
                    }
                    else
                    {
                        int index = code - XK_peep;

                        keysym_entry * entry = &shavian_to_keysym[ index ];

                        send_key( to_keysym( entry->keysym1 ), entry->keysym2 );
                    }
                }
            }

        } while ( utf8_str.get_next( code ) );
    }
}

void
C_x11_output::send( key_event_t key_event, uint8_t scancode )
{
    //TEMP
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "C_x11_output::send()- key_event: %04xh, scancode: %04xh"
                                               , key_event
                                               , scancode );

    // Check for the scancode used for the Roman/Shavian alphabet switch
    if ( scancode == KC_EXECUTE )
    { 
        if ( key_event == KEY_EV_DOWN )
        {
            toggle_shavian();
        }

        return;
    }
    
    if ( scancode > 0x7f )
    {
        return;
    }

    // Do a keysym lookup based on the standard (non-shavian) layout
    KeySym keysym = scancode_to_keysym[ scancode ];

    //TEMP
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "ACRO: scancode: %04xh, keysym: %04xh", scancode, keysym  );

    if ( is_shift( keysym ) )
    {
        shift_ = ( key_event == KEY_EV_DOWN );
    
        if ( shift_ != shift_prev_ )
        {
            shift_prev_ = shift_;
        }
    }

    if ( shavian_ )
    {
        //TEMP
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "ACRO: %s", is_shavian_key( keysym) ? "Is shavian key" : "NOT shavian key"  );

        if ( is_shavian_key( keysym ) )
        {
            // We need a second lookup (which will replace the roman layout keysym) to get the shavian keysym
            unsigned long index = keysym - XK_A;

            keysym = to_keysym( shavian_keysym[ index ] );
        
            //TEMP
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "ACRO: index: %ld, keysym: %04xh", index, keysym  );
        }
    }

    if ( keysym != 0 )
    {
        KeyCode keycode = XKeysymToKeycode( display_, keysym );
     
        //TEMP
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "ACRO: keycode: %04xh", keycode );
        
        if ( keycode != 0 )
        {
            XTestGrabControl( display_, True );

            // Generate regular key press and release
            if ( key_event == KEY_EV_DOWN )
            {
                log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "keysym: %04xh keycode: %04xh (key down)", keysym, keycode );
                XTestFakeKeyEvent( display_, keycode, True, 0 );
            }
            else if ( key_event == KEY_EV_UP )
            {
                log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "keysym: %04xh keycode: %04xh (key up)", keysym, keycode );
                XTestFakeKeyEvent( display_, keycode, False, 0 ); 
            } 
         
            XSync( display_, False );
            XTestGrabControl( display_, False );
        }
    }
}

void
C_x11_output::stop()
{
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
    int index = 0;

    for ( const char ** entry = XF86_symstrings; *entry; entry++ )
    {
        keysym_entry * ks_entry = new keysym_entry(); 

        ks_entry->keysym1 = shavian_keysyms[ index ].keysym1;
        ks_entry->keysym2 = shavian_keysyms[ index ].keysym2;

        keysym_replacements_->insert( std::make_pair( *entry, *ks_entry ) );
        index++;
    }
};

void
C_x11_output::set_shavian_keysyms()
{
    //TEMP
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "set_shavian_keysyms()" );

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

    // Loop through the keycodes and look for keysyms associated with each keycode
    // that can be set to use Shavian keysyms instead.
    // XF86_symstrings contains a list of keysym strings which seem like good candidates
    // for re-use (they will become unavailable for their original purpose).

    bool done = false;

    for ( int keycode = keycode_low; ( keycode <= keycode_high ) && ( ! done ); keycode++ )
    {
        int sym_index = ( ( keycode - keycode_low ) * keysyms_per_keycode );

        KeySym keysym = keysyms[ sym_index ];

        const char * keysymstring = XKeysymToString( keysym ); 

        //TEMP
        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "keysymstring: %s", keysymstring );
        
        if ( keysymstring != nullptr )
        {
            auto result = keysym_replacements_->find( keysymstring );

            if ( result != keysym_replacements_->end() )
            {
                log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "keycode: %04xh, str: %s, ks1: %05xh, ks2: %05xh"
                                                           , keycode
                                                           , keysymstring
                                                           , to_keysym( result->second.keysym1 )
                                                           , to_keysym( result->second.keysym2 ) );

                // Found an entry we can repurpose for Shavian
                KeySym keysym_list[] = { to_keysym( result->second.keysym1 )
                                       , to_keysym( result->second.keysym2 )
                                       , NoSymbol
                                       , NoSymbol
                                       , NoSymbol
                                       , NoSymbol
                                       , NoSymbol };

                //TODO Save original keymap so we can restore it on program exit
                XChangeKeyboardMapping( display_, keycode, keysyms_per_keycode, keysym_list, 1 );
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

keysym_entry
C_x11_output::shavian_to_keysym[] =
{
    { XK_peep,         0          }
,   { XK_tot,          0          }
,   { XK_kick,         0          }
,   { XK_fee,          0          }
,   { XK_thigh,        0          }
,   { XK_so,           0          }
,   { XK_sure,         0          }
,   { XK_church,       0          }
,   { XK_yea,          0          }
,   { XK_hung,         0          }
,   { XK_bib,          0          }
,   { XK_dead,         XK_Shift_L }
,   { XK_gag,          XK_Shift_L }
,   { XK_vow,          XK_Shift_L }
,   { XK_they,         XK_Shift_L }
,   { XK_zoo,          XK_Shift_L }

,   { XK_measure,      XK_Shift_L }
,   { XK_judge,        XK_Shift_L }
,   { XK_woe,          XK_Shift_L }
,   { XK_haha,         XK_Shift_L }
,   { XK_loll,         0          }
,   { XK_mime,         0          }
,   { XK_if,           0          }
,   { XK_egg,          0          }
,   { XK_ash,          0          }
,   { XK_ado,          0          }
,   { XK_on,           0          }
,   { XK_wool,         0          }
,   { XK_out,          0          }
,   { XK_ah,           0          }
,   { XK_roar,         XK_Shift_L }
,   { XK_none,         XK_Shift_L }

,   { XK_eat,          XK_Shift_L }
,   { XK_age,          XK_Shift_L }
,   { XK_ice,          XK_Shift_L }
,   { XK_up,           XK_Shift_L }
,   { XK_oak,          XK_Shift_L }
,   { XK_ooze,         XK_Shift_L }
,   { XK_oil,          XK_Shift_L }
,   { XK_awe,          XK_Shift_L }
,   { XK_are,          0          }
,   { XK_or,           XK_Shift_L }
,   { XK_air,          0          }
,   { XK_urge,         XK_Shift_L }
,   { XK_array,        0          }
,   { XK_ear,          XK_Shift_L }
,   { XK_ian,          0          }
,   { XK_yew,          XK_Shift_L }
};

// Shavian keysyms in their unshifted form
const KeySym
C_x11_output::shavian_keysym[] =
{
                    // ASCII
    XK_egg          // a
,   XK_ian          // b
,   XK_kick         // c
,   XK_are          // d
,   XK_wool         // e
,   XK_fee          // f
,   XK_array        // g
,   XK_hung         // h 
,   XK_ash          // i
,   XK_church       // j 
,   XK_ah           // k
,   XK_loll         // l
,   XK_mime         // m
,   XK_thigh        // n
,   XK_on           // o 
,   XK_peep         // p
,   XK_yea          // q
,   XK_out          // r
,   XK_so           // s
,   XK_tot          // t
,   XK_ado          // u
,   XK_sure         // v 
,   XK_air          // w
,   XK_namingdot    // x
,   XK_if           // y
,   XK_acroring     // z
};

const keysym_entry
C_x11_output::shavian_keysyms[] =
{
    { XK_ash,       XK_ice       }
,   { XK_ian,       XK_yew       }
,   { XK_kick,      XK_gag       }
,   { XK_are,       XK_or        }
,   { XK_egg,       XK_age       }
,   { XK_fee,       XK_vow       }
,   { XK_array,     XK_ear       }
,   { XK_so,        XK_zoo       }
,   { XK_if,        XK_eat       }
,   { XK_church,    XK_judge     }
,   { XK_ah,        XK_awe       }
,   { XK_loll,      XK_roar      }
,   { XK_mime,      XK_none      }
,   { XK_thigh,     XK_they      }
,   { XK_on,        XK_oak       }
,   { XK_peep,      XK_bib       }
,   { XK_yea,       XK_woe       }
,   { XK_out,       XK_oil       }
,   { XK_sure,      XK_measure   }
,   { XK_tot,       XK_dead      }
,   { XK_ado,       XK_up        }
,   { XK_hung,      XK_haha      }
,   { XK_air,       XK_urge      }
,   { XK_namingdot, XK_namingdot }
,   { XK_wool,      XK_ooze      }
,   { XK_acroring , XK_acroring  }
};

// Array of symkey strings whose references to keycodes in the keyboard
// will have Shavian code point substituted.
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
,   nullptr
};

void
C_x11_output::test()
{
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "C_x11_output::test" );

    unsigned long len;
    char *        window_name;
    
    Window * window_list = ( Window * ) list( display_, &len );


    for ( int ii = 0; ii < ( int ) len; ii++ )
    {
        window_name = C_x11_output::name( display_, window_list[ ii ] );

        fprintf( stdout, "%d: %s \n", ii, window_name );

        free( window_name );
    }




    //Window focused = None;
    //int revert_to  = 0;

    //XGetInputFocus( display_, &focused, &revert_to );

    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "focused: %08lx", focused );

    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%s", find_window_handle( focused )
                                                     //? "WE HAVE FOCUS!"
                                                     //: "we don't have focus" );

#if 0
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
                code = to_keysym( code );
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
#endif
}

bool
C_x11_output::find_window_handle( Window search_wnd )
{
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "find_window_handle" );

    bool res = find_window_handle_2( XDefaultRootWindow( display_ ), search_wnd );

    return res;
}

bool
C_x11_output::find_window_handle_2( Window current_wnd, Window search_wnd )
{
    Window   root     = None;
    Window   parent   = None;
    Window * children = None;
 
    unsigned children_count = 0;
  
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "current_wnd: %08lx, search_wnd: %08lx", current_wnd, search_wnd );

    // Check if window handle matches
    if ( current_wnd == search_wnd )
    {
        return true;
    }

    bool result = false;
    
    // If not, check all subwindows recursively
    if ( XQueryTree( display_, current_wnd, &root, &parent, &children, &children_count ) != 0 )
    {
        for ( unsigned int ii = 0; ii < children_count; ii++ )
        {
            result = find_window_handle_2( children[ ii ], search_wnd );
            
            if ( result )
            {
                break;
            }
        }

        XFree( children );
    }

    return result;
}

Window *
C_x11_output::list( Display * disp, unsigned long * len )
{
    int             form;
    unsigned long   remain;
    unsigned char * list;
    
    Atom prop = XInternAtom( disp,"_NET_CLIENT_LIST", False );
    Atom type;

    if ( XGetWindowProperty( disp, XDefaultRootWindow( disp ), prop, 0, 1024, False, XA_WINDOW,
                             &type, &form, len, &remain, &list) != Success )
    {
        return 0;
    }

    return ( Window * ) list;
}

char *
C_x11_output::name( Display * disp, Window win )
{
    int             form;
    unsigned long   len;
    unsigned long   remain;
    unsigned char * list;

    Atom prop = XInternAtom( disp, "WM_NAME", False);
    Atom type;

    //if ( XGetWindowProperty( disp, win, prop, 0, 1024, False, XA_STRING,
    if ( XGetWindowProperty( disp, win, prop, 0, 1024, False, AnyPropertyType,
                             &type, &form, &len, &remain, &list) != Success )
    {
        return NULL;
    }

    return ( char * ) list;
}

}

#endif // X11
