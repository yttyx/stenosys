// window_scroll.h

#ifndef    window_scroll_I
#define    window_scroll_I

#include <ncurses.h>

#include "window.h"

using namespace std;

namespace aksaramustika
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
    write( const string & message );

protected:


private:

};

}

#endif    // window_scroll_I
