// strokefeed.cpp

#include <iostream>
#include <fstream>
#include <stdio.h>

#include "log.h"
#include "strokefeed.h"

#define LOG_SOURCE "SFEED"

using namespace stenosys;

namespace stenosys
{
extern C_log log;

C_stroke_feed::C_stroke_feed()
{
}

C_stroke_feed::~C_stroke_feed()
{
}

bool
C_stroke_feed::check_file()
{
    text_stream_.seekg( std::ios_base::beg );

    std::string steno;

    while ( read( steno ) )
    {
        if ( ! ( steno.find_first_not_of( "#STKPWHRAO*EUFRPBLGTSDZ-" ) == std::string::npos ) )
        {
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Non-steno text in file: %s ", steno.c_str() );
            return false;
        }
    }

    return true;
}

}
