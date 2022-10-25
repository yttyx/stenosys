// window.cpp
  
#include <errno.h>
//#include <stdio.h>
#include <string.h>
//#include <stdlib.h>

#include "log.h"
#include "window.h"


namespace stenosys
{

extern C_log log;


C_ncurses::C_ncurses()
{
    if ( instance_count++ == 0 )
    {
        initscr();      // Initialise ncurses
        cbreak();       // Line buffering disabled
    }
}

C_ncurses::~C_ncurses()
{
    if ( --instance_count == 0 )
    {
        endwin();
    }
}

unsigned int C_ncurses::instance_count = 0;

C_window::C_window( const char * title
                  , int          top
                  , int          left
                  , int          height
                  , int          width )
{
    top_main_    = top;
    left_main_   = left;
    height_main_ = height;
    width_main_  = width;

    top_         = top_main_ + 1; 
    left_        = left_main_ + 1;
    height_      = height_main_ - 2;
    width_       = width_main_ - 2;

    row_curr_    = 0;
    col_curr_    = 0;

    wnd_main_    = newwin( height_main_, width_main_, top_main_, left_main_ );
    wnd_         = newwin( height_, width_, top_, left_ );
	
	box( wnd_main_, 0 , 0 );    // 0, 0 gives default characters for the vertical and horizontal lines

    if ( strlen( title ) > 0 )
    {
        wmove( wnd_main_, 0, 1 );
        waddch(wnd_main_, ' ' );
        waddstr(wnd_main_, title );
        waddch(wnd_main_, ' ' );
    }

    curs_set( 0 );              // Make cursor invisible
	wrefresh( wnd_main_ );		// Display the box
    wrefresh( wnd_ );           // Display the the window
}

C_window::~C_window()
{
	/* The parameters taken are 
	 * 1. win: the window on which to operate
	 * 2. ls: character to be used for the left side of the window 
	 * 3. rs: character to be used for the right side of the window 
	 * 4. ts: character to be used for the top side of the window 
	 * 5. bs: character to be used for the bottom side of the window 
	 * 6. tl: character to be used for the top left corner of the window 
	 * 7. tr: character to be used for the top right corner of the window 
	 * 8. bl: character to be used for the bottom left corner of the window 
	 * 9. br: character to be used for the bottom right corner of the window
	 */
	wborder( wnd_main_, ' ', ' ', ' ',' ',' ',' ',' ',' ' );

	wrefresh( wnd_ );
	delwin( wnd_ );

	wrefresh( wnd_main_ );
	delwin( wnd_main_ );
}

void
C_window::write( const std::string & message )
{
    wmove( wnd_, row_curr_, col_curr_ );
    wclrtoeol( wnd_ );

    wprintw( wnd_, "%s", message.c_str() );
    wrefresh( wnd_ );
}

void
C_window::clear()
{
    wclear( wnd_ );

    row_curr_ = 0;
    col_curr_ = 0;
}

}
