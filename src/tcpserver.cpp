#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdio.h>

#include "log.h"
#include "miscellaneous.h"
#include "tcpserver.h"

#define LOG_SOURCE "TCPSV"

using namespace  stenosys;

namespace stenosys
{

extern C_log log;

C_tcp_server::C_tcp_server()
    : socket_( -1 )
    , client_( -1 )
    , port_( -1 )
    , abort_( false )
    , running_( false )
{
    ip_buffer_ = std::make_unique< C_buffer< char > >();
    op_buffer_ = std::make_unique< C_buffer< char > >();
}

C_tcp_server::~C_tcp_server()
{
    cleanup();
}

bool
C_tcp_server::initialise( int port )
{
    port_ = port;

    struct sockaddr_in server_sockaddr;

    socket_ = socket( PF_INET, SOCK_STREAM, 0 );
   
    if ( socket_ == -1 )
    {
        errfn_ = "socket()";
        errno_ = errno;
        return false;
    }

    socklen_t option_length = 1;

    int rc = setsockopt( socket_, SOL_SOCKET, SO_REUSEADDR, &option_length, sizeof( option_length ) );
    
    if ( rc == -1 )
    {
        errfn_ = "setsocketopt()";
        errno_ = errno;
        cleanup();
        return false;
    }
   
    // initialize the server's sockaddr
    memset( &server_sockaddr, 0, sizeof( server_sockaddr ) );

    server_sockaddr.sin_family      = AF_INET;
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_sockaddr.sin_port        = htons( port_ );
  
    rc = bind( socket_, ( struct sockaddr * ) &server_sockaddr, sizeof( server_sockaddr ) );
  
    if ( rc == -1 )
    {
        errfn_ = "bind()";
        errno_ = errno;
        cleanup();
        return false;
    }

    rc = listen( socket_, 10 );
  
    if ( rc == -1 )
    {
        errfn_ = "listen()";
        errno_ = errno;
        cleanup();
        return false;
    }
  
    return true;
}

bool
C_tcp_server::start()
{
    running_ = true;

    return thread_start();
}

void
C_tcp_server::stop()
{
    abort_ = true;
    thread_await_exit();
}

bool
C_tcp_server::running()
{
    return running_;
}

bool
C_tcp_server::send_text( const std::string & message )
{
    int rc = send( client_, message.c_str(), message.length(), 0 );

    if ( rc == -1 )
    {
        errfn_ = "send()";
        errno_ = errno;
        return false;
    }

    return true;
}

bool
C_tcp_server::get_line( std::string & line )
{
    //TEMP echo character via the ring buffers/thread handler
   
    char ch = '\0';

    while ( true )
    {
        if ( ip_buffer_->get( ch ) )
        {
            if ( ch == 'q' )
            {
                // we're done
                break;
            }

            op_buffer_->put( ch );
        }

        delay( 1 );
    }

    return false;
}

void
C_tcp_server::cleanup()
{
    if ( socket_ != -1 )
    {
        close( socket_ );
    }
   
    if ( client_ != -1 )
    {
        close( client_ );
    }
}

// -----------------------------------------------------------------------------------
// Background thread code
// -----------------------------------------------------------------------------------

void
C_tcp_server::thread_handler()
{
    while ( ! abort_ )
    {
        if ( ! got_client_connection() )
        {
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%s error %d", errfn_.c_str(), errno_ );
            break;
        }

        if ( ! send_banner() )
        {
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%s error %d", errfn_.c_str(), errno_ );
        }
      
        int  rc    = -1;
        char ip_ch = '\0';
        char op_ch = '\0';

        while ( true )
        {
            // Receive one character (non-blocking)
            rc = recv( client_, &ip_ch, 1, 0 );
            
            // ( rc == -1 ) && ( ( errno == EAGAIN ) || ( rc == EWOULDBLOCK ) ) if no data is available
            if (  rc != -1 )
            {
                if ( ip_ch == 0x04 )
                {
                    // EOT: terminate client connection
                    int rc = close( client_ );

                    client_ = -1;

                    if ( rc != -1 )
                    {
                        errfn_ = "close()";
                        errno_ = errno;
                    }

                    // Go back and wait for another incoming connection                    
                    break;
                }
                else
                {
                    // A character is ready
                    ip_buffer_->put( ip_ch );
                }
            }

            // If any data is available in the output buffer, send it
            if ( op_buffer_->get( op_ch ) )
            {
                rc = send( client_, &op_ch, 1, 0 );
            }

            delay( 1 );
        }
    }
    
    running_ = false;
}

bool
C_tcp_server::got_client_connection()
{
    struct sockaddr_in client_sockaddr;
    socklen_t          client_sockaddr_size = sizeof( sockaddr_in );

    client_ = accept( socket_, ( struct sockaddr * ) &client_sockaddr, &client_sockaddr_size );
  
    if ( client_ == -1 )
    {
        errfn_ = "accept()";
        errno_ = errno;
        return false;
    }
  
    // Set client socket as non-blocking
    fcntl( client_, F_SETFL, O_NONBLOCK );
    
    return true;
}

bool
C_tcp_server::send_banner()
{
    int rc = send( client_, "stenosys\r\n", 10, 0 );

    if ( ( rc == -1 ) || ( rc != 10 ) )
    {
        errfn_ = "banner send()";
        errno_ = errno;
        return false;
    }

    return true;
}

}
