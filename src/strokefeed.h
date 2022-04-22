//strokefeed.h
#pragma once

#include <cstddef>
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
    get_steno( std::string & steno );

private:

    bool
    check_file();

    bool
    parse_line( const std::string & line, const char * regex, std::string & param );
    
    bool
    convert();

private:

    std::unique_ptr< std::vector< S_geminipr_packet > > packets_; 

    std::unique_ptr< std::vector< std::string > > strokes_; 
    std::vector< std::string >::iterator          strokes_it_;
    //size_t strokes_size_;

    };

}
