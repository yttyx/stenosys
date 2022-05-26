// x11output.h
#pragma once

#include "keyevent.h"
#include "stdarg.h"
#include <X11/X.h>
#include <cstdint>
#include <string>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>
#include <vector>

#include "outputter.h"

namespace stenosys
{

const uint32_t SHAVIAN_10450        = 0x10450;
const uint32_t SHAVIAN_10450_KEYSYM = 0x1010450;
const uint32_t SHAVIAN_1047f        = 0x1047f;
const uint32_t SHAVIAN_MDOT         = 0x00B7;

struct keysym_entry
{
    KeySym base;
    KeySym modifier;
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

private:

    static keysym_entry ascii_to_keysym[];
    static keysym_entry ascii_to_shavian_keysym[];
    static const char * XF86_symstrings[];
    static KeySym       scancode_to_keysym[];

    Display * display_;

    std::vector< std::string > symstrings_;

};

}
