#pragma once

#include "mutex.h"
#include "thread.h"
#include "tcpserver.h"

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

    void
    find( const std::string & word );

private:
    
    void
    thread_handler();

private:

    bool abort_;
    bool port_;

    std::unique_ptr< C_tcp_server > tcpserver_;
};

}
