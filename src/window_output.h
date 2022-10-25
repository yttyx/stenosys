// window_output.h
#pragma once

#include <ncurses.h>
#include <string>

#include "window.h"


namespace stenosys
{

class C_window_output : public C_window
{

public:

    C_window_output( const char *title
                   , int        top
                   , int        left
                   , int        height
                   , int        width );
    
    virtual ~C_window_output();

    virtual void
    write( const std::string & message );

protected:


private:

    std::string window_text_;

};

}
