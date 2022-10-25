// window.h
#pragma once

#include <string>

#include <ncurses.h>


namespace stenosys
{

class C_ncurses
{

public:

    C_ncurses(); 
    ~C_ncurses(); 

private:

    static bool         initialised;
    static unsigned int instance_count;

};

class C_window : public C_ncurses
{

public:

    C_window( const char *title
            , int        top
            , int        left
            , int        height
            , int        width );
    
    virtual ~C_window();

    virtual void
    write( const std::string & message );

    virtual void
    clear();

protected:


protected:

    WINDOW *wnd_;                 // Client area, the area within the main window

    int top_main_;                // Main window
    int left_main_;
    int height_main_;
    int width_main_; 

    int top_;                     // Client area window
    int left_;
    int height_;
    int width_; 

    int row_curr_;
    int col_curr_;

private:

    WINDOW *wnd_main_;            // Main window, which includes its border

};

}
