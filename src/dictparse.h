#pragma once

#include <iostream>

namespace stenosys
{

class C_command_parser
{

enum case_convert_type { CCT_UPPERCASE_LETTER, CCT_UPPERCASE_WORD, CCT_LOWERCASE_LETTER, CCT_LOWERCASE_WORD };

public:
    C_command_parser();
    ~C_command_parser();

    void
    parse( const std::string & text_in, std::string & text_out, uint16_t & flags );
    
    const std::string
    get_error_message();

protected:

    bool
    contains( const char * regex, const std::string & text );

    void
    parse_command( const std::string & text_in, std::string & text_out, uint16_t & flags );
    
    void
    parse_raw_input( const std::string & text_in, std::string & text_out, uint16_t & flags );

    void
    process_command( const std::string & command, std::string & text_out, uint16_t & flags );

    void
    process_text( std::string text_in, std::string & text_out, uint16_t & flags );

    void
    change_case( case_convert_type conversion_type, std::string & text_in );

    void
    lookup_raw( const std::string & raw_command, std::string & text_out, uint16_t & flags );

private:

};

}