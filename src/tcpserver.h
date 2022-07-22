#pragma once

#include <string>

#include "mutex.h"
#include "thread.h"

namespace stenosys
{

class C_tcp_server : public C_thread
{
public:

    C_tcp_server();
    ~C_tcp_server();

    bool
    initialise( int port );

    bool
    start();

    void
    stop();

    bool 
    send_text( const std::string & message );

private:

    void
    thread_handler();

    bool
    got_client_connection();

    bool
    echo_characters();
    
    void
    cleanup();

private:

    int  socket_;
    int  client_;
    int  port_;
    bool abort_;

    std::string errfn_;
    int         errno_;
};

}
