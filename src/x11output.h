// x11output.h
#pragma once

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

const uint32_t SHAVIAN_10450 = 0x10450;
const uint32_t SHAVIAN_1047f = 0x1047f;
const uint32_t SHAVIAN_MDOT  = 0x00B7;

const int SHAVIAN_TABLE_SIZE = ( int ) ( SHAVIAN_1047f - SHAVIAN_10450 + 1 );



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

    int shavian_table_index( KeySym code );

private:

    static keysym_entry ascii_to_keysym[];

    Display * display_;

    std::vector< std::string > symstrings_;

    int shavian_keycodes_[ SHAVIAN_TABLE_SIZE ];
};

}
