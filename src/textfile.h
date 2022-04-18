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

    std::string
    get_error_message() { return error_message_; }

protected:

    std::stringstream  text_stream_;
    std::string        error_message_;
};

}
