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

using namespace  stenosys;

namespace stenosys
{

extern C_log log;

C_tcp_server::C_tcp_server()
    : listener_( -1 )
    , client_( -1 )
    , port_( -1 )
    , abort_( false )
    , running_( false )
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

    fds_[ 0 ].fd = listener_;
    fds_[ 0 ].events = POLLIN;

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
    bool close_connection = false;

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

                do
                {
                    // Receive data on this connection until the recv() fails with  EWOULDBLOCK. If any
                    // other failure occurs, close the connection.
                    
                    rc = recv( fds_[ fds_idx ].fd, buffer, sizeof( buffer ), 0 );

                    if ( rc < 0 )
                    {
                        if ( errno != EWOULDBLOCK )
                        {
                            log_writeln( C_log::LL_INFO, LOG_SOURCE, "recv() failed" );
                            abort_ = true;
                        }

                        break;
                    }

                    if ( rc == 0 )
                    {
                        log_writeln( C_log::LL_INFO, LOG_SOURCE, "Connection closed" );
                        
                    }
                } while ();
            }
        }

     


        char ip_ch = '\0';
        char op_ch = '\0';

        while ( ! abort_ )
        {
            // Receive one character (non-blocking)
            rc = recv( client_, &ip_ch, 1, 0 );
            
            // ( rc == -1 ) && ( ( errno == EAGAIN ) || ( rc == EWOULDBLOCK ) ) if no data is available
            if (  rc != -1 )
            {
                if ( ip_ch == 0x04 )
                {
                    // EOT: terminate client connection
                    close_client();

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

    // Close client socket (if open) and server socket
    cleanup();

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Shutting down %s socket server thread", banner_.c_str() );
    
    running_ = false;
}

//bool
//C_tcp_server::got_client_connection()
//{
    //struct sockaddr_in client_sockaddr;
    //socklen_t          client_sockaddr_size = sizeof( sockaddr_in );

    //client_ = accept( listener_, ( struct sockaddr * ) &client_sockaddr, &client_sockaddr_size );
  
    //if ( client_ == -1 )
    //{
        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "accept() error %d", errno );
        //return false;
    //}
  
    //if ( ( client_ == -1 ) || ( rc == -1 ) )
    //{
        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%s error %d", errfn_.c_str(), errno_ );
    //}

    //return true;
//}

bool
C_tcp_server::send_banner()
{
    int rc  = send( client_, banner_.c_str(), banner_.length(), 0 );
    int rc2 = send( client_, "\r\n", 2, 0 );

    if ( ( rc == -1 ) || ( rc2 == -1 ) )
    {
        errfn_ = "banner send()";
        errno_ = errno;
        return false;
    }

    return true;
}

void
C_tcp_server::cleanup()
{
    if ( listener_ != -1 )
    {
        close( listener_ );
        listener_ = -1;
    }
   
    if ( client_ != -1 )
    {
        close( client_ );
        client_ = -1;
    }
}

void
C_tcp_server::close_client()
{
    if ( client_ != -1 )
    {
        close( client_ );
        client_ = -1;
    }
}

}
