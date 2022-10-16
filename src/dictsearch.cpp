// dictsearch.cpp

#include <memory>
#include <stdio.h>
#include <string>

#include "dictsearch.h"
#include "log.h"
#include "miscellaneous.h"

#define LOG_SOURCE "SERCH"

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_dictionary_search::C_dictionary_search()
    : abort_( false)
    , port_( -1 )
{
}

bool
C_dictionary_search::initialise( int port )
{
    tcpserver_ = std::make_unique< C_tcp_server >();

    return tcpserver_->initialise( port, "Dictionary search" );
}

bool
C_dictionary_search::start()
{
    return tcpserver_->start() && thread_start();
}

void
C_dictionary_search::stop()
{
    tcpserver_->stop();

    abort_ = true;
    thread_await_exit();
}

void
C_dictionary_search::find( const std::string & word )
{
    //TBW
}

// -----------------------------------------------------------------------------------
// Background thread code
// -----------------------------------------------------------------------------------

void
C_dictionary_search::thread_handler()
{
    while ( ! abort_ )
    {
        char ch = '\0';

        if ( tcpserver_->get_char( ch ) )
        {
            switch ( ch )
            {
                case '\r':
                case '\n':
                    // Line terminator
                    if ( search_string_.length() > 0 )
                    {
                        tcpserver_->put_text( std::string( "\r\n" ) );
                        dict_search( search_string_ );
                        search_string_.clear();
                    }
                    break;

                case '\b':
                    // Backspace
                    if ( search_string_.length() > 0 )
                    {
                        search_string_.pop_back();
                        tcpserver_->put_text( std::string( "\b" ) );
                    }                    
                    break;
                
                case 0x04:
                    // Ctrl-D: terminate session
                    abort_ = true;
                    break;
                
                default:
                    // Check for alphanumeric character. If not alphanumeric, or the limit
                    // of the search string length has been reached, then ignore it.
                    if ( isalnum( ch ) )
                    {
                        if ( search_string_.length() < SEARCH_STRING_MAX )
                        {
                            search_string_ += ch;
                            tcpserver_->put_char( ch );
                        }
                    }
                    break;
            }
        }
        else
        {
            delay( 1 );
        }
    }
}

void
C_dictionary_search::dict_search( const std::string & search_string_ )
{
}

}
