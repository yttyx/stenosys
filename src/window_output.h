// window_output.h

#ifndef    window_output_I
#define    window_output_I

#include <ncurses.h>
#include <string>

#include "window.h"

using namespace std;

namespace aksaramustika
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
    write( const string & message );

protected:


private:

    string      window_text_;

};

}

#endif    // window_output_I
