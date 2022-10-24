// window_scroll.cpp
  
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "log.h"
#include "window_scroll.h"


using namespace std;

namespace aksaramustika
{

extern C_log log;


C_window_scroll::C_window_scroll( const char *title
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

C_window_scroll::~C_window_scroll()
{
}

void
C_window_scroll::write( const string & message )
{
    // Move to next row, or stay on bottom row and scroll
    if ( row_curr_ >= ( height_ ) )
    {
        scroll( wnd_ );
        mvwprintw( wnd_, height_ - 1, col_curr_, "%s", message.c_str() );
    }
    else
    {
        mvwprintw( wnd_, row_curr_, col_curr_, "%s", message.c_str() );
    }
    wrefresh( wnd_ );

    row_curr_ = min( row_curr_ + 1, height_ );
}

}
