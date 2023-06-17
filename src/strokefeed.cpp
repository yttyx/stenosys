// strokefeed.cpp

#include <exception>
#include <iostream>
#include <fstream>
#include <memory>
#include <regex>
#include <stdio.h>

#include "geminipr.h"
#include "log.h"
#include "miscellaneous.h"
#include "strokefeed.h"


using namespace stenosys;

namespace stenosys
{

extern C_log log;

// Sample line:
//   SR-R     // very
//   TAOEURD  // tired
//
// '-' in REGEX_STROKE must be at the beginning or end of the [] section so it's not 
// interpreted as a range (and gives the error "Invalid range in bracket expression")
const char * REGEX_STROKE = "\\s*([#STKPWHRAO*EUFBLGDZ*-]+)";

C_stroke_feed::C_stroke_feed()
{
    strokes_ = std::make_unique< std::vector< std::string > >(); 
}

C_stroke_feed::~C_stroke_feed()
{
    log_writeln( C_log::LL_INFO, "C_stroke_feed destructor" );
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

        while ( C_text_file::get_line( line ) && ( line != "end" ) )
        {
            if ( parse_line( line, REGEX_STROKE, steno ) )
            {
                if ( steno.find_first_not_of( "#STKPWHRAO*EUFBLGDZ-" ) == std::string::npos )
                {
                    strokes_->push_back( steno );
                }
                else
                {
                    log_writeln_fmt( C_log::LL_INFO, "Non-steno text in stroke file: %s ", line.c_str() );
                    worked = false;
                }
            }
            else
            {
                log_writeln_fmt( C_log::LL_INFO, "Failed to parse line: %s ", line.c_str() );
                worked = false;
            }
        }
    }

    // Free up memory from the vector container now we know how much we need
    strokes_->shrink_to_fit();
    
    strokes_it_ = strokes_->begin();

    return worked;
}
    
bool
C_stroke_feed::get_steno( std::string & steno )
{
    if ( strokes_it_ != strokes_->end() )
    {
        steno = *strokes_it_;
        strokes_it_++;

        delay( 100 );

        return true;
    }

    return false;
}

bool
C_stroke_feed::parse_line( const std::string & line, const char * regex, std::string & param )
{
    try
    {
        std::regex regex_entry( regex );

        std::smatch matches;

        if ( std::regex_search( line, matches, regex_entry ) )
        {
            std::ssub_match match = matches[ 1 ];

            param = match.str();

            return true;
        }
    }
    catch ( std::exception & ex )
    {
        log_writeln_fmt( C_log::LL_INFO, "Stroke feed: exception parsing line %s: %s ", line.c_str(), ex.what() );
    }    

    return false;
}

}
