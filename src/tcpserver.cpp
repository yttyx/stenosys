#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdio.h>

#include "log.h"
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

    socket_ = socket( PF_INET,SOCK_STREAM, 0 );
   
    if ( socket_ == -1 )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "socket() failed, error %d", errno );
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

        if ( ! send_text( "stenosys\r\n" ) )
        {
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%s error %d", errfn_.c_str(), errno_ );
            return;
        }
        
        if ( ! echo_characters() )
        {
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%s error %d", errfn_.c_str(), errno_ );
            break;
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

    return true;
}

bool 
C_tcp_server::echo_characters()
{

    char input = '\0';
    int  rc    = -1;

    while ( true )
    {
        rc = recv( client_, &input, 1, 0 );
        
        if ( rc == -1 )
        {
            errfn_ = "recv()";
            errno_ = errno;
            break;
        }

        if ( input == 'x' )
        {
            break;
        }

        if ( input == 'q' )
        {
            abort_ = true;
            break;
        }

        rc = send( client_, &input, 1, 0 );
        
        if ( rc == -1 )
        {
            errfn_ = "send()";
            errno_ = errno;
            break;
        }
    }

    int rc2 = close( client_ );

    client_ = -1;

    if ( ( rc != -1 ) && ( rc2 == -1 ) )
    {
        errfn_ = "close()";
        errno_ = errno;
    }

    return ( rc != -1 ) && ( rc2 != -1 );
}

}
