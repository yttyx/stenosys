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
const char * REGEX_STROKE = "\\s*([\\w+\\-]+)";


C_stroke_feed::C_stroke_feed()
{
//    packets_ = std::make_unique< std::vector< S_geminipr_packet > >(); 
    strokes_ = std::make_unique< std::vector< std::string > >(); 
}

C_stroke_feed::~C_stroke_feed()
{
}

bool
C_stroke_feed::initialise( const std::string & filepath )
{
    bool worked = true;

    if ( C_text_file::read( filepath ) )
    {
        text_stream_.seekg( std::ios_base::beg );

        std::string line;
        std::string steno;

        log_writeln( C_log::LL_INFO, LOG_SOURCE, "initialise(): 1" );

        while ( C_text_file::get_line( line ) && ( line != "end" ) )
        {
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "line: %s", line.c_str() );

            if ( parse_line( line, REGEX_STROKE, steno ) )
            {
                if ( steno.find_first_not_of( "#STKPWHRAO*EUFRPBLGTSDZ-" ) == std::string::npos )
                {
                    strokes_->push_back( steno );
                    //packets_->push_back( *C_gemini_pr::encode( steno ) );
                }
                else
                {
                    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Non-steno text in stroke file: %s ", line.c_str() );
                    worked = false;
                }
            }
            else
            {
                log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Failed to parse line: %s ", line.c_str() );
                worked = false;
            }
        }
    }

    // Free up memory from the vector container now we know how much we need
    strokes_->shrink_to_fit();
    
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "got here"  );

    //strokes_size_ = strokes_->size();
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "strokes_size_: %u", strokes_->size() );

    strokes_it_ = strokes_->begin();



    //std::string temp = ( *strokes_)[ 0 ];
    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "first steno: %s ", ( *strokes_)[ 0 ].c_str() );

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "first steno: %s ", ( *strokes_it_).c_str() );

    return worked;
}
    
bool
C_stroke_feed::get_steno( std::string & steno )
{
    //if ( strokes_size_ < strokes_->size() )
    if ( strokes_it_ != strokes_->end() )
    {

        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "steno: %s ", steno.c_str() );

        //strokes_size_++;
        strokes_it_++;

        return true;
    }

    return false;
}

bool
C_stroke_feed::parse_line( const std::string & line, const char * regex, std::string & param )
{
    std::regex regex_entry( regex );

    std::smatch matches;

    //const char * REGEX_STROKE = "^\\s*(\\S+?)\\s*$";

    if ( std::regex_search( line, matches, regex_entry ) )
    {
        std::ssub_match match = matches[ 1 ];

        param = match.str();

        return true;
    }

    return false;
}

}
