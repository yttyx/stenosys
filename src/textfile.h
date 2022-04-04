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

    std::string
    get_error_message() { return error_message_; }

    bool
    parse_line( const std::string & line, const char * regex, std::string & param1, std::string & param2 );

protected:

    std::string       error_message_;
};

}
