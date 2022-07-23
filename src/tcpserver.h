#pragma once

#include <memory>
#include <string>

#include "mutex.h"
#include "buffer.h"
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
    running();

    bool 
    send_text( const std::string & message );

    bool
    get_line( std::string & line );

private:

    void
    thread_handler();

    bool
    got_client_connection();
    
    bool
    send_banner();

    void
    cleanup();

private:

    int  socket_;
    int  client_;
    int  port_;
    bool abort_;
    bool running_;
    bool connected__;

    std::string errfn_;
    int         errno_;
    
    std::unique_ptr< C_buffer< char > > ip_buffer_;
    std::unique_ptr< C_buffer< char > > op_buffer_;

};

}
