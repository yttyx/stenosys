// x11output.h
#pragma once

#include "keyevent.h"
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

#include "outputter.h"

namespace stenosys
{

#define is_modifier( x )    ( ( XK_Shift_L <= ( x ) ) && ( ( x ) <= XK_Hyper_R ) )
#define is_shift( x )       ( ( ( x ) == XK_Shift_L ) || ( x == XK_Shift_R ) )
#define is_shavian_key( x ) ( ( XK_A <= ( x ) ) && ( x <= XK_Z ) )
#define to_keysym( x )      ( ( ( x ) >= XK_peep ) ? ( ( x ) + 0x1000000 ) : x )

struct keysym_entry
{
    KeySym base;
    KeySym modifier;
};

class shavian_keysym_entry
{

public:

    shavian_keysym_entry( KeySym base, KeySym shifted ) { base_ = base; shifted_ = shifted; }

    KeySym base_;
    KeySym shifted_;
};

class C_x11_output : public C_outputter
{

public:

    C_x11_output();
    ~C_x11_output();

    virtual bool    
    initialise();

    virtual void
    send( const std::string & str );

    virtual void
    send( key_event_t key_event, uint8_t scancode );

    virtual void
    test();

private:

    void
    set_up_data();
    
    void
    find_unused_keycodes();

    //virtual std::string
    //format( const std::string & text );
    
    void 
    send_key( KeySym keysym, KeySym modsym );

    void
    send_key( KeyCode keycode );
    
    virtual void
    toggle_shavian();

private:
    
    bool shavian_;  // mode for typed output:   
                    //   true:  shavian alphabet
                    //   false: latin alphabet
    bool shift_;    // used in typed Shavian mode
    bool shift_prev_;    // used in typed Shavian mode

    Display * display_;

    std::unique_ptr< std::unordered_map< std::string, shavian_keysym_entry > > keysym_replacements_;
    
    std::vector< std::string > symstrings_;

    static keysym_entry ascii_to_keysym[];
    static keysym_entry ascii_to_shavian_keysym[];
    static const char * XF86_symstrings[];
    static KeySym       scancode_to_keysym[];
    static KeySym       shavian_keysym[];

    static const keysym_entry the_table[];
};

}
