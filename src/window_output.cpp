// window_output.cpp
  
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "log.h"
#include "window_output.h"


using namespace std;

namespace aksaramustika
{

extern C_log log;


C_window_output::C_window_output( const char *title
                                , int        top
                                , int        left
                                , int        height
                                , int        width )
    : C_window( title, top, left, height, width )
{
    wsetscrreg( wnd_, top_, height_ );
    
    idlok( wnd_, false );
    scrollok( wnd_, true );

    wmove( wnd_, row_curr_, col_curr_ );
}

C_window_output::~C_window_output()
{
}

void
C_window_output::write( const string & message )
{
    // Just keep concatenating text
    window_text_ += message;    

    mvwprintw( wnd_, row_curr_, col_curr_, "%s", window_text_.c_str() );
    wrefresh( wnd_ );
}

}
