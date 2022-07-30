#pragma once

#include "tcpserver.h"

namespace stenosys
{

class C_dictionary_search
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
    

private:

    bool port_;

    std::unique_ptr< C_tcp_server > tcpserver_;
};

}
