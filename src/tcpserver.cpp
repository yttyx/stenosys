#include <cstdint>
#include <cstring>    // sizeof()
#include <iostream>
#include <stdio.h>
#include <string>   

// headers for socket(), getaddrinfo() and friends
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <unistd.h>    // close()

#include "tcpserver.h"

using namespace  stenosys;

namespace stenosys
{

C_tcp_server::C_tcp_server()
    : port_( 0 )
{
}


bool
C_tcp_server::initialise( int port )
{
    port_ = port;

    //int serverPort = 5000;
    struct sockaddr_in serverSa;

    int s  = socket( PF_INET,SOCK_STREAM, 0 );
   
    int on = 1;
    int rc = setsockopt( s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on );
    
    // initialize the server's sockaddr
    memset( &serverSa, 0, sizeof( serverSa ) );

    serverSa.sin_family = AF_INET;
    serverSa.sin_addr.s_addr = htonl(INADDR_ANY);
    serverSa.sin_port = htons( port_ );
  
    rc = bind( s,( struct sockaddr * ) &serverSa, sizeof( serverSa ) );
  
    if ( rc < 0 )
    {
        perror( "bind failed" );
        return false;
    }

    rc = listen( s, 10 );
  
    if ( rc < 0 )
    {
        perror("listen failed");
        return false;
    }
  
    socklen_t          clientSaSize;
    struct sockaddr_in clientSa;

    int c = accept( s,( struct sockaddr * ) &clientSa, &clientSaSize );
  
    if ( c < 0 )
    {
        perror( "accept failed" );
        return false;
    }
  
    //printf("Client address is: 
    rc = write( c, "hello world\n", 12 );
  
    close( c );
    close( s );

    return true;
}

}
