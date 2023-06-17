// x11output.h
#pragma once

#include "stdarg.h"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <X11/X.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>
#include <vector>

#include "keyevent.h"

namespace stenosys
{

#define is_modifier( x )    ( ( XK_Shift_L <= ( x ) ) && ( ( x ) <= XK_Hyper_R ) )
#define is_shift( x )       ( ( ( x ) == XK_Shift_L ) || ( x == XK_Shift_R ) )
#define is_shavian_key( x ) ( ( XK_A <= ( x ) ) && ( x <= XK_Z ) )

#define is_shavian_code( x ) ( ( ( XK_peep <= ( x ) ) && ( x <= XK_yew ) ) || \
                               ( x == XK_namingdot ) || \
                               ( x == XK_acroring  ) )

// From keysymdef.h:
// "For any future extension of the keysyms with characters already
//  found in ISO 10646 / Unicode, the following algorithm shall be
//  used. The new keysym code position will simply be the character's
//  Unicode number plus 0x01000000. The keysym values in the range
//  0x01000100 to 0x0110ffff are reserved to represent Unicode"
//
// 0x10450 is the base value of the Shavian code block

#define to_keysym( x ) ( ( ( ( x ) >= XK_peep ) || ( ( x ) == XK_acroring ) ) ? ( ( x ) + 0x1000000 ) : x )


struct keysym_entry
{
    keysym_entry()
    {
        keysym1 = 0;
        keysym2 = 0;
    }
    
    keysym_entry( KeySym ks1, KeySym ks2 )
    {
        keysym1 = ks1;
        keysym2 = ks2;
    }

    KeySym keysym1;
    KeySym keysym2;
};

class C_x11_output
{

public:

    C_x11_output();
    ~C_x11_output();

    virtual bool    
    initialise();

    virtual bool    
    initialise( const std::string & output_device );
    
    virtual void
    send( const std::string & str );

    virtual void
    send( key_event_t key_event, uint8_t scancode );

    virtual void
    test();
    
    virtual void
    stop();

    void
    set_keymapping();

private:

    void
    set_up_data();
    
    void
    set_shavian_keysyms();
    
    void
    backup_keysyms();

    void
    restore_keysyms();

    void 
    send_key( KeySym keysym, KeySym modsym );

    void
    send_key( KeyCode keycode );
    
    virtual void
    toggle_shavian();

    bool
    find_window_handle( Window search_wnd );

    bool
    find_window_handle_2( Window current_wnd, Window search_wnd );

    Window *
    list ( Display * disp, unsigned long * len );

    char *
    name( Display * disp, Window win );

private:
    
    bool shavian_;      // mode for typed output:   
                        //   true : Shavian alphabet
                        //   false: Roman alphabet
    bool shift_;        // used in typed Shavian mode
    bool shift_prev_;   // used in typed Shavian mode

    Display * display_;
    KeySym *  orig_keysyms_;
    
    int keysyms_per_keycode_;
    int keycode_low_;
    int keycode_high_;

    int orig_keysyms_per_keycode_;
    int orig_keycode_low_;
    int orig_keycode_high_;
    
    std::unique_ptr< std::unordered_map< std::string, keysym_entry > > keysym_replacements_;
    
    std::vector< std::string > symstrings_;
    

    static keysym_entry ascii_to_keysym[];
    static keysym_entry shavian_to_keysym[];

    static keysym_entry ascii_to_shavian_keysym[];
    static const char * XF86_symstrings[];
    static KeySym       scancode_to_keysym[];

    static const KeySym       shavian_keysym[];
    static const keysym_entry shavian_keysyms[];

};

}
