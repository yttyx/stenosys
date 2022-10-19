// dictsearch.cpp

#include <memory>
#include <stdio.h>
#include <string>

#include "dictionary_i.h"
#include "dictsearch.h"
#include "log.h"
#include "miscellaneous.h"

#define LOG_SOURCE    "SERCH"
#define SEARCH_PROMPT "Find: "

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

// -----------------------------------------------------------------------------------
// Background thread code
// -----------------------------------------------------------------------------------

void
C_dictionary_search::thread_handler()
{
    std::list< std::string > search_results;

    tcpserver_->put_text( SEARCH_PROMPT );

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
                        tcpserver_->put_text( "\r\n" );

                        search_results.clear();
                        word_lookup( search_string_, 20, search_results );

                        search_results.sort();
                        report( search_results );
                        
                        search_string_.clear();
                                
                        tcpserver_->put_text( SEARCH_PROMPT );
                    }
                    break;

                case '\b':
                    // Backspace
                    if ( search_string_.length() > 0 )
                    {
                        search_string_.pop_back();
                        tcpserver_->put_text( std::string( "^h" ) );
                    }                    
                    break;
                
                //case 0x04:
                    //// Ctrl-D: terminate session
                    //abort_ = true;
                    //break;
                
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
C_dictionary_search::report( std::list< std::string> results )
{
    for ( std::string entry : results )
    {
        tcpserver_->put_text( entry + "\r\n" );
    }
}

}
