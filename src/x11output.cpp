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

#include "log.h"
#include "miscellaneous.h"
#include "utf8.h"
#include "x11output.h"

#define LOG_SOURCE "X11OP"

using namespace stenosys;

namespace stenosys
{

extern C_log log;


C_x11_output::C_x11_output()
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
                // 0x10450 is the base value of the Shavian code block
                if ( code >= SHAVIAN_10450 )
                {
                    code += 0x1000000;
                }

                send_key( code, 0 );
            }

        } while ( utf8_str.get_next( code ) );
    }
}

void
C_x11_output::send( uint16_t scancode )
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
    for ( const char ** entry = XF86_symstrings; *entry; entry++ )
    {
        symstrings_.push_back( std::string( *entry ) );
    }
}

void
C_x11_output::find_unused_keycodes()
{
    // Check whether a Shavian KeySym has already been set up
    int keycode = XKeysymToKeycode( display_, SHAVIAN_10450_KEYSYM );
 
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

    uint32_t shavian = SHAVIAN_10450;

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

                        if ( shavian == SHAVIAN_MDOT )
                        {
                            // We're done
                            done = true;
                        }
                        else {

                            shavian++;

                            if ( shavian > SHAVIAN_1047f )
                            {
                                // Shavian alphabet is done, just need to set the middle dot
                                // in the next available slot.
                                shavian = SHAVIAN_MDOT;
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
