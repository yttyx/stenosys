// x11output.h
#pragma once

#include "stdarg.h"
#include <string>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>

namespace stenosys
{

class C_x11_output
{

public:

    C_x11_output();
    ~C_x11_output();

    bool    
    initialise();

    void
    test();

private:

    void 
    send_key( KeySym keysym, KeySym modsym );

private:

    Display * display_;

};

}
