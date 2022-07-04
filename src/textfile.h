#pragma once

#include <string>
#include <sstream>

namespace stenosys
{

class C_text_file
{
public:

    C_text_file();
    ~C_text_file();

    bool
    read( const std::string & path );

    bool
    get_line( std::string & line );

protected:

    std::stringstream  text_stream_;
};

}
