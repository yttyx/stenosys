#pragma once

#include <sys/poll.h>

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
    initialise( int port, const char * banner );

    bool
    start();

    void
    stop();

    bool
    running();

    bool 
    send_text( const std::string & message );

    bool
    get_line( std::string & line, int max_length );

// Foreground
private:

    void
    cleanup();

// Background thread
private:

    void
    thread_handler();

    bool
    send_banner();

private:

    int  port_;
    bool abort_;
    bool running_;
   
    int           listener_;
    int           fds_count_;
    struct pollfd fds_[ 3 ];

    std::string banner_;

    std::unique_ptr< C_buffer< char > > ip_buffer_;
    std::unique_ptr< C_buffer< char > > op_buffer_;

};

}
