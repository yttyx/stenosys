// window_scroll.h
#pragma once

#include <ncurses.h>
#include <string>

#include "window.h"


namespace stenosys
{

class C_window_scroll : public C_window
{

public:

    C_window_scroll( const char *title
                   , int        top
                   , int        left
                   , int        height
                   , int        width );
    
    virtual ~C_window_scroll();

    virtual void
    write( const std::string & message );

protected:


private:

};

}
