#pragma once

#include "mutex.h"
#include "thread.h"
#include "tcpserver.h"

#include <list>
#include <string>

#define SEARCH_STRING_MAX  16

namespace stenosys
{

class C_dictionary_search : C_thread
{

public:
    
    C_dictionary_search();
    ~C_dictionary_search() {}

    bool
    initialise( int port );

    bool
    start();

    void
    stop();

private:
    
    void
    thread_handler();

    void
    report( std::list< std::string> results );

private:

    bool abort_;
    bool port_;

    std::string search_string_;

    std::unique_ptr< C_tcp_server > tcpserver_;
};

}
