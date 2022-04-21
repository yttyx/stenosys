// strokefeed.cpp

#include <iostream>
#include <fstream>
#include <memory>
#include <regex>
#include <stdio.h>

#include "geminipr.h"
#include "log.h"
#include "strokefeed.h"

#define LOG_SOURCE "SFEED"

using namespace stenosys;

namespace stenosys
{

extern C_log log;

// Sample line:
//   SR-R     // very
//   TAOEURD  // tired
const char * REGEX_STROKE = "^\\s*(\\S+?)\\s*$";

C_stroke_feed::C_stroke_feed()
{
    packets_ = std::make_unique< std::vector< S_geminipr_packet > >(); 
}

C_stroke_feed::~C_stroke_feed()
{
}

bool
C_stroke_feed::initialise( const std::string & filepath )
{
    bool worked = true;

    text_stream_.seekg( std::ios_base::beg );

    std::string line;
    std::string steno;

    while ( read( line ) )
    {
        if ( parse_line( line, REGEX_STROKE, steno ) )
        {
            if ( steno.find_first_not_of( "#STKPWHRAO*EUFRPBLGTSDZ-" ) == std::string::npos )
            {
                packets_->push_back( *C_gemini_pr::encode( steno ) );
            }
            else
            {
                log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Non-steno text in stroke file: %s ", line.c_str() );
                worked = false;
            }
        }
        else
        {
            worked = false;
        }
    }

    return worked;
}

bool
C_stroke_feed::parse_line( const std::string & line, const char * regex, std::string & param )
{
    std::regex regex_entry( regex );

    std::smatch matches;

    if ( std::regex_search( line, matches, regex_entry ) )
    {
        std::ssub_match match = matches[ 1 ];

        param = match.str();

        return true;
    }

    return false;
}

}
