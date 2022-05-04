// x11output.h
#pragma once

#include "stdarg.h"
#include <X11/X.h>
#include <string>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>

#include "outputter.h"

namespace stenosys
{

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

    virtual std::string
    format( const std::string & text );
    
    void 
    send_key( KeySym keysym, KeySym modsym );

private:

    Display * display_;

    static keysym_entry ascii_to_keysym[];

};

}
