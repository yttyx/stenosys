#include <arpa/inet.h>
#include <asm-generic/errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
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

// Ref: https://www.ibm.com/docs/en/i/7.1?topic=designs-using-poll-instead-select

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_tcp_server::C_tcp_server()
    : port_( -1 )
    , abort_( false )
    , running_( false )
    , listener_( -1 )
    , fds_count_( 1 )
{
    ip_buffer_ = std::make_unique< C_buffer< char > >();
    op_buffer_ = std::make_unique< C_buffer< char > >();
}

C_tcp_server::~C_tcp_server()
{
    cleanup();
}

bool
C_tcp_server::initialise( int port, const char * banner )
{
    port_   = port;
    banner_ = banner;

    struct sockaddr_in server_sockaddr;

    listener_ = socket( PF_INET, SOCK_STREAM, 0 );
   
    if ( listener_ == -1 )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "socket() error %d", errno );
        return false;
    }

    socklen_t option_length = 1;

    int rc = setsockopt( listener_, SOL_SOCKET, SO_REUSEADDR, &option_length, sizeof( option_length ) );
    
    if ( rc == -1 )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "setsocketopt() error %d", errno );
        cleanup();
        return false;
    }
  
    // Set socket to be nonblocking. All of the sockets for
    // the incoming connections will also be nonblocking since
    // they will inherit that state from the listening socket
    int on = 0;

    rc = ioctl( listener_, FIONBIO, ( char * ) &on );

    if ( rc < 0 )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "ioctl() error %d", errno );
        cleanup();
        return false;
    }

    // Initialize the server's sockaddr
    memset( &server_sockaddr, 0, sizeof( server_sockaddr ) );

    server_sockaddr.sin_family      = AF_INET;
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_sockaddr.sin_port        = htons( port_ );
  
    rc = bind( listener_, ( struct sockaddr * ) &server_sockaddr, sizeof( server_sockaddr ) );
  
    if ( rc == -1 )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "bind() error %d", errno );
        cleanup();
        return false;
    }

    rc = listen( listener_, 10 );
  
    if ( rc == -1 )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "listen() error %d", errno );
        cleanup();
        return false;
    }

    // Set up the listening socket
    memset( fds_, 0 , sizeof( fds_ ) ); 

    fds_[ 0 ].fd     = listener_;
    fds_[ 0 ].events = POLLIN;
    fds_count_ = 1;

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%s server listening on port %d", banner_.c_str(), port_ );
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

// TBW Write a block of data out directly - protect shared buffer with a mutex
bool
C_tcp_server::send_text( const std::string & message )
{
    for ( char ch : message )
    {
        op_buffer_->put( ch );
    }

    op_buffer_->put( '\r' );
    op_buffer_->put( '\n' );

    return true;
}

bool
C_tcp_server::get_line( std::string & line )
{
    //TEMP
    line = "";

    //char ch = '\0';

    //while ( true )
    //{
        //if ( ip_buffer_->get( ch ) )
        //{
            //if ( ch == 'q' )
            //{
                //// we're done
                //abort_ = true;
                //break;
            //}

            //op_buffer_->put( ch );
        //}

        //delay( 1 );
    //}

    return false;
}

// -----------------------------------------------------------------------------------
// Background thread code
// -----------------------------------------------------------------------------------

void
C_tcp_server::thread_handler()
{
    int  rc               = -1;

    while ( ! abort_ )
    {
        // Poll socket/s with a timeout of 200mS
        rc = poll( fds_, fds_count_, 200 );

        if ( rc == -1 )
        {
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "poll() error %d, terminating thread", errno );
            break;
        }

        if ( rc == 0 )
        {
            // Poll timed out. Check for abort and carry on.
            continue;
        }

        for ( int fds_idx = 0; fds_idx < fds_count_; fds_idx++ )
        {
            if ( fds_[ fds_idx ].revents != POLLIN )
            {
                log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "revents value error %d", fds_[ fds_idx ].revents );
                abort_ = true; 
                break;
            }
     
            if ( fds_[ fds_idx ].fd == listener_ )
            {
                int new_client = -1; 

                do
                {
                    log_writeln( C_log::LL_INFO, LOG_SOURCE, "listening socket is readable" );
                   
                    new_client = accept( listener_, nullptr, nullptr );

                    if ( new_client < 0 )
                    {
                        if ( errno != EWOULDBLOCK )
                        {
                            log_writeln( C_log::LL_INFO, LOG_SOURCE, "accept() failed" );
                            abort_ = true;
                        }

                        break;
                    }

                    // If we already have one connection made, reject this new connection
                    if  ( fds_count_ >= 2 )
                    {
                        close( new_client );

                        log_writeln( C_log::LL_INFO, LOG_SOURCE, "Aleady have one connection; rejected new client" );
                        break;
                    }

                    // Add the incoming connection to the fds_ array
                    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "New incoming connection %d", new_client );

                    fds_[ fds_count_ ].fd     = new_client;
                    fds_[ fds_count_ ].events = POLLIN;
                    fds_count_++;              

                } while ( new_client != -1 );
            }
            else
            {
                // Not the listening socket, therefore an existing socket must be readable
                log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Descriptor %d is readable", fds_[ fds_idx ] );
              
                char buffer[ 256 ];

                bool close_connection = false;

                while ( true ) 
                {
                    // Receive data on this connection until the recv() fails with  EWOULDBLOCK. If any
                    // other failure occurs, close the connection.
                    
                    rc = recv( fds_[ fds_idx ].fd, buffer, sizeof( buffer ), 0 );

                    if ( rc < 0 )
                    {
                        if ( errno != EWOULDBLOCK )
                        {
                            log_writeln( C_log::LL_INFO, LOG_SOURCE, "recv() failed" );
                            close_connection = true; 
                        }

                        break;
                    }

                    if ( rc == 0 )
                    {
                        log_writeln( C_log::LL_INFO, LOG_SOURCE, "Connection closed" );
                        close_connection = true;
                        break;
                    }

                    int len = rc;

                    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Data received: %d bytes", len  );

                    // Put the received data into the ring buffer
                    for ( int ii = 0; ii < len; ii++ )
                    {
                        ip_buffer_->put( buffer[ ii ] );
                    }
                }

                if ( close_connection )
                {
                    close( fds_[ fds_idx ].fd );
                    fds_[ fds_idx ].fd = -1;
                    fds_count_--;
                }
            }
        }
    }
    
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Shutting down %s socket server thread", banner_.c_str() );

    running_ = false;
}     

bool
C_tcp_server::send_banner()
{
    //TODO
    //int rc  = send( client_, banner_.c_str(), banner_.length(), 0 );
    //int rc2 = send( client_, "\r\n", 2, 0 );

    //if ( ( rc == -1 ) || ( rc2 == -1 ) )
    //{
        //errfn_ = "banner send()";
        //errno_ = errno;
        //return false;
    //}

    return true;
}

void
C_tcp_server::cleanup()
{
    for ( int ii = 0; ii < fds_count_; ii++ )
    {
        close( fds_[ ii ].fd );
        fds_[ ii ].fd = -1;
    }
    
    fds_count_ = 1;
}

}
