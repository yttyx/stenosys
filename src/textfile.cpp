// dictionary.cpp

#include <iostream>
#include <fstream>
#include <stdio.h>

#include "log.h"
#include "textfile.h"

#define LOG_SOURCE "READF"

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_text_file::C_text_file()
{
}

C_text_file::~C_text_file()
{
}

bool
C_text_file::read( const std::string & path )
{
    bool worked = false;

    try
    {
        std::ifstream file_stream( path, std::ios::in | std::ios::binary );

        if ( file_stream.is_open() )
        {
            // Read the whole file into text_
            text_stream_ << file_stream.rdbuf();
     
            file_stream.close();

            worked = true;
        }
        else
        {
            log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "**Error reading file %s", path.c_str() );
        }
    }
    catch ( std::exception & ex )
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "**File read exception: %s", ex.what() );
    }
    catch( ... )
    {
    }

    return worked;
}

bool
C_text_file::get_line( std::string & line )
{
    if ( text_stream_.eof() )
    {
        return false;
    }

    std::getline( text_stream_, line, '\n' );

    return true;
}

}
