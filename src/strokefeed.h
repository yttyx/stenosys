//strokefeed.h
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "geminipr.h"
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

    bool
    parse_line( const std::string & line, const char * regex, std::string & param );

private:

    bool
    check_file();

    bool
    convert();

private:

    std::unique_ptr< std::vector< S_geminipr_packet > > packets_; 
};

}
