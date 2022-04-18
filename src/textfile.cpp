// dictionary.cpp

#include <iostream>
#include <fstream>
#include <stdio.h>

#include "textfile.h"


using namespace stenosys;

namespace stenosys
{

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
            error_message_ = "**Error reading file";
        }
    }
    catch ( std::exception & ex )
    {
        error_message_ = ex.what();
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
