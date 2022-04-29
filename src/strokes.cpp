
#include <algorithm>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "common.h"
#include "history.h"
#include "log.h"
#include "strokes.h"

#define LOG_SOURCE "STRKS"

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_strokes::C_strokes()
{
    stroke_history_ = std::make_unique< C_history< std::string, 10 > >();
}

C_strokes::~C_strokes()
{
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "C_strokes destructor" );
}

void
C_strokes::add( const std::string & steno )
{
    stroke_history_->add( steno );
}

bool
C_strokes::get_current( std::string & steno )
{
    if ( stroke_history_->get_current( steno ) )
    {
        steno_curr_ = steno;
    
        return true;
    }

    return false;
}

bool
C_strokes::get_previous( std::string & steno )
{
    std::string steno_prev;

    if ( stroke_history_->get_previous( steno_prev ) )
    {
        steno       = steno_prev + std::string( "/" ) + steno_curr_;
        steno_curr_ = steno;
        return true;
    }

    return false;
}

void
C_strokes::clear()
{
    stroke_history_->clear();
}

#if 0
void
C_strokes::dump()
{
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "  steno         translation       flgs  sq  s'ceded" );
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "  ------------  ----------------  ----  --  -------" );

    C_stroke * stroke = stroke_curr_;

    for ( std::size_t ii = 0; ii < STROKE_BUFFER_MAX; ii++ )
    {
        std::string translation = stroke->get_translation();

        std::string trans_field;

        if ( translation.length() > 0 )
        {
            trans_field = std::string( "|" ) + ctrl_to_text( translation ) + std::string( "|" );
        }

        char line[ 2048 ];

        snprintf( line, sizeof( line ), "%s%-12.12s  %-16.16s  %04x  %2d  %-7.7s"
                                      , ( ii == 0 ) ? "* " : "  "
                                      , stroke->get_steno().c_str()
                                      , trans_field.c_str()
                                      , stroke->get_flags()
                                      , stroke->get_seqnum()
                                      , stroke->get_superceded() ? "true" : "false" );
   
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%s", line );

        stroke = stroke->get_prev();
    }
}
#endif

#if 0
// Convert control characters in a string to text
// For example, "\n" becomes [0a]"
std::string
C_strokes::ctrl_to_text( const std::string & text )
{
    std::string output;

    for ( size_t ii = 0; ii < text.length(); ii++ )
    {
        if ( iscntrl( text[ ii ] ) )
        {
            char buffer[ 10 ];

            snprintf( buffer, sizeof( buffer ), "[%02x]", text[ ii ] );
            output += buffer;
        }
        else
        {
            output += text[ ii ];
        }
    }

    return output;
}
#endif 

}
