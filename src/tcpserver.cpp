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
    : socket_( 0 )
    , client_( 0 )
    , port_( 0 )
{
}

bool
C_tcp_server::initialise( int port )
{
    port_ = port;

    struct sockaddr_in server_sockaddr;

    int socket_ = socket( PF_INET,SOCK_STREAM, 0 );
   
    if ( socket_ == -1 )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "socket() failed, error %d", errno );
        return false;
    }

    socklen_t option_length = 1;

    int rc = setsockopt( socket_, SOL_SOCKET, SO_REUSEADDR, &option_length, sizeof option_length );
    
    if ( rc == -1 )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "setsocketopt() failed, error %d", errno );
        close( socket_ );
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
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "bind() failed, error %d", errno );
        close( socket_ );
        return false;
    }

    rc = listen( socket_, 10 );
  
    if ( rc == -1 )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "listen() failed, error %d", errno );
        close( socket_ );
        return false;
    }
  
    socklen_t          client_sockaddr_size;
    struct sockaddr_in client_sockaddr;

    log_writeln( C_log::LL_INFO, LOG_SOURCE, "Waiting for incoming connection" );

    int client_ = accept( socket_, ( struct sockaddr * ) &client_sockaddr, &client_sockaddr_size );
  
    if ( client_ == -1 )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "accept() failed, error %d", errno );
        close( socket_ );
        return false;
    }

    log_writeln( C_log::LL_INFO, LOG_SOURCE, "Connection successful" );
    
    //printf("Client address is: 
    std::string msg = "hello\r\n";
    rc = send( client_, msg.c_str(), msg.length(), 0 );

    msg = "type characters; <esc> to exit\r\n";
    rc  = send( client_, msg.c_str(), msg.length(), 0 );

    char input = '\0';

    while ( true )
    {
        rc = recv( client_, &input, 1, 0 );
        
        if ( rc < 1 )
        {
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "TCP read error %d", rc );
            break;
        }

        if ( input == 0x1b )
        {
            break;
        }

        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Received %c [%04x]", input, input );

        rc = send( client_, &input, 1, 0 );
        
        if ( rc < 1 )
        {
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "TCP write error %d", rc );
            break;
        }
    }

    log_writeln( C_log::LL_INFO, LOG_SOURCE, "End of echo loop" );

    rc = close( client_ );

    if ( rc == -1 )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "close( client_ ) error %d", rc );
    }

    rc = close( socket_ );
    
    if ( rc == -1 )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "close( socket_ ) error %d", rc );
    }

    return true;
}

}
