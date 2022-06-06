#pragma once

//#include <cstdint>
#include <iostream>
#include <memory>
#include <unordered_map>

#include "textfile.h"

namespace stenosys
{


class C_shavian_dictionary : C_text_file
{
public:

    C_shavian_dictionary();
    ~C_shavian_dictionary();

    bool
    read( const std::string & path );

    bool
    lookup( const std::string & latin, std::string & shavian );

private:

    bool
    parse_line( const std::string & line, const char * regex, std::string & param1, std::string & param2 );

private:

    bool initialised_;

    std::string error_message_;
    
    std::unique_ptr< std::unordered_map< std::string, std::string > >  dictionary_;

};

}
