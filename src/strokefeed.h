//strokefeed.h
#pragma once

#include <string>

#include "textfile.h"

namespace stenosys
{

class C_stroke_feed : C_text_file
{
public:

    C_stroke_feed();
    ~C_stroke_feed();

    bool
    initialise( const std::string & filepath );

private:

    bool
    check_file();

};

}
