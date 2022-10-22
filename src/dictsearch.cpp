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
    , sent_prompt_( false )
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
        if ( ! sent_prompt_ )
        {
            tcpserver_->put_text( SEARCH_PROMPT );
            sent_prompt_ = true;
        }

        char ch = '\0';

        if ( tcpserver_->get_char( ch ) )
        {
            //TEMP
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "recv: [%02x]", ch );

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
                        sent_prompt_ = false;
                    }
                    break;

                case '\x7f':
                    // Backspace
                    if ( search_string_.length() > 0 )
                    {
                        search_string_.pop_back();
                        tcpserver_->put_text( std::string( "\x7f" ) );
                    }                    
                    break;
                
                default:
                    // Check for valid search string character. If the limit
                    // of the search string length has been reached then ignore it.
                    if ( VALID_SEARCH_CHAR( ch ) )
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
