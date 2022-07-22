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
   
    socklen_t option_length = 1;

    int rc = setsockopt( socket_, SOL_SOCKET, SO_REUSEADDR, &option_length, sizeof option_length );
   
    // initialize the server's sockaddr
    memset( &server_sockaddr, 0, sizeof( server_sockaddr ) );

    server_sockaddr.sin_family      = AF_INET;
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_sockaddr.sin_port        = htons( port_ );
  
    rc = bind( socket_, ( struct sockaddr * ) &server_sockaddr, sizeof( server_sockaddr ) );
  
    if ( rc < 0 )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "bind() failed, error %d", rc );
        return false;
    }

    rc = listen( socket_, 10 );
  
    if ( rc < 0 )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "listen() failed, error %d", rc );
        return false;
    }
  
    socklen_t          client_sockaddr_size;
    struct sockaddr_in client_sockaddr;

    log_writeln( C_log::LL_INFO, LOG_SOURCE, "Waiting for incoming connection" );

    int client_ = accept( socket_, ( struct sockaddr * ) &client_sockaddr, &client_sockaddr_size );
  
    if ( client_ < 0 )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "accept() failed, error %d", rc );
        return false;
    }

    //printf("Client address is: 
    rc = write( client_, "hello world\n", 12 );
  
    close( client_ );
    close( socket_ );

    return true;
}

}
