#include <arpa/inet.h>
#include <cstddef>
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

// Ref:  https://www.ibm.com/docs/en/i/7.1?topic=designs-using-poll-instead-select
//       https://stackoverflow.com/questions/12170037/when-to-use-the-pollout-event-of-the-poll-c-function

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

    banner_ =  "\r\n";
    banner_ += banner;
    banner_ += "\r\n";

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
  
    // Set socket to be nonblocking
    int on = 1;

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
C_tcp_server::put_text( const std::string & text )
{
    return op_buffer_->put_block( text.c_str(), text.length() );
}

bool
C_tcp_server::put_char( char ch )
{
    return op_buffer_->put(ch );
}

bool
C_tcp_server::get_line( std::string & line, int max_length )
{
    line = "";

    char ch = '\0';

    while ( true )
    {
        if ( ip_buffer_->get( ch ) )
        {
            if ( ( ch == '\r' ) || ( ch == '\n' ) )
            {
                break;
            }
            
            line += ch;
            
            if ( line.length() >= ( size_t ) max_length )
            {
                break;
            }
        }
        else 
        {
            delay( 1 );
        }
    }

    return line.length() > 0;
}

bool
C_tcp_server::get_char( char & ch )
{
    return ip_buffer_->get( ch );
}

// -----------------------------------------------------------------------------------
// Background thread code
// -----------------------------------------------------------------------------------

void
C_tcp_server::thread_handler()
{
    int rc = -1;

    //log_writeln( C_log::LL_INFO, LOG_SOURCE, "Starting TCP server thread" );
    
    while ( ! abort_ )
    {
        // Check whether we have a client connected and there is data to send
        if ( ( fds_count_ == 2 ) && ( op_buffer_->count() > 0 ) )
        {
            //log_writeln( C_log::LL_INFO, LOG_SOURCE, "got send data" );

            // Set event flag so we get notified next time poll() is called
            fds_[ 1 ].events |= POLLOUT;
        }
        
        // Poll socket/s with a timeout of 100mS
        rc = poll( fds_, fds_count_, 100 );

        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Return from poll() %d", rc );
        
        if ( rc == -1 )
        {
            log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "poll() error %d, terminating thread", errno );
            break;
        }

        if ( rc == 0 )
        {
            // Poll timed out. Check for abort and carry on.
            continue;
        }

        // fds_count_ may increase if there's an incoming connection, but we must not
        // include a new entry into the event checks loop until poll() above has been called.
        int fds_count_curr = fds_count_;

        for ( int fds_idx = 0; fds_idx < fds_count_curr; fds_idx++ )
        {
            //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "fds_idx: %d", fds_idx );

            if ( fds_[ fds_idx ].revents == 0 )
            {
                continue;
            }
            
            char ch     = '\0';
            int  send_len = 0;

            if ( fds_[ fds_idx ].revents & POLLIN )
            { 
                if ( fds_[ fds_idx ].fd == listener_ )
                {
                    int new_client = -1; 

                    do
                    {
                        //log_writeln( C_log::LL_INFO, LOG_SOURCE, "listening socket is readable" );
                       
                        new_client = accept( listener_, nullptr, nullptr );

                        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "new_client: %d, errno: %d", new_client, errno );
                        
                        if ( new_client < 0 )
                        {
                            if ( errno != EWOULDBLOCK )
                            {
                                log_writeln( C_log::LL_ERROR, LOG_SOURCE, "accept() failed" );
                                abort_ = true;
                            }

                            break;
                        }

                        // If we already have one connection made, reject this new connection
                        if  ( fds_count_ >= 2 )
                        {
                            close( new_client );

                            log_writeln( C_log::LL_ERROR, LOG_SOURCE, "Aleady have one connection; rejected new client" );
                            break;
                        }

                        // Make the client socket nonblocking
                        int on = 1;

                        rc = ioctl( new_client, FIONBIO, ( char * ) &on );

                        if ( rc < 0 )
                        {
                            log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "new client ioctl() error %d", errno );
                            abort_ = true;
                            break;
                        }

                        // Add the incoming connection to the fds_ array
                        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "New incoming connection %d", new_client );

                        fds_[ fds_count_ ].fd     = new_client;
                        fds_[ fds_count_ ].events = POLLIN;
                        fds_count_++;              
    
                        // Cue up banner message
                        op_buffer_->put_block( banner_.c_str(), banner_.length() );

                    } while ( new_client != -1 );
                }
                else
                {
                    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Descriptor %d is readable", fds_[ fds_idx ] );
                  
                    char buffer[ 256 ];

                    bool close_connection = false;

                    while ( true ) 
                    {
                        // Receive data on this connection until the recv() fails with EWOULDBLOCK. If any
                        // other failure occurs, close the connection.
                        
                        //log_writeln( C_log::LL_INFO, LOG_SOURCE, "before recv()" );
                        
                        rc = recv( fds_[ fds_idx ].fd, buffer, sizeof( buffer ), 0 );

                        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "recv() returned %d", rc  );
                        
                        if ( rc < 0 )
                        {
                            if ( errno != EWOULDBLOCK )
                            {
                                log_writeln( C_log::LL_ERROR, LOG_SOURCE, "recv() failed" );
                                close_connection = true; 
                            }

                            //log_writeln( C_log::LL_INFO, LOG_SOURCE, "EWOULDBLOCK" );
                            break;
                        
                        }
                        else if ( rc == 0 )
                        {
                            log_writeln( C_log::LL_INFO, LOG_SOURCE, "Connection closed" );
                            close_connection = true;
                            break;
                        }
                        else
                        {
                            int len = rc;

                            //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Data received: %d bytes", len  );

                            // Put the received data into the ring buffer
                            for ( int ii = 0; ii < len; ii++ )
                            {
                                ip_buffer_->put( buffer[ ii ] );
                            }
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

            if ( fds_[ fds_idx ].revents & POLLOUT )
            { 
                //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "op_buffer_->count(): %d", op_buffer_->count() );
                
                char buffer[ 256 ];

                send_len = 0;

                for ( size_t ii = 0; ii < sizeof( buffer ); ii++ )
                {
                    if ( op_buffer_->get( ch ) )
                    {
                        buffer[ ii ] = ch;
                        send_len++;
                    }
                    else
                    {
                        break;
                    }
                }

                //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "before send(), send_len: %d", send_len );

                rc = send( fds_[ fds_idx ].fd, buffer, send_len, 0 );
                
                if ( rc < 0 )
                {
                   log_writeln( C_log::LL_ERROR, LOG_SOURCE, "send() failed" );
                }
        
                // Clear event flag. NB: this is based on the assumption that all the data
                // was successfully sent. Review TBD.
                fds_[ 1 ].events &= ( ~POLLOUT );
            }
            
            if ( fds_[ fds_idx ].revents & ( ~ ( POLLIN | POLLOUT ) ) )
            {
                  log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "unexpected event %d", fds_[ fds_idx ].revents );
                  abort_ = true; 
                  break;
            }
        }
    }
    
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Shutting down '%s' socket server thread", banner_.c_str() );

    running_ = false;
}     

void
C_tcp_server::cleanup()
{
    for ( int ii = 0; ii < fds_count_; ii++ )
    {
        if ( fds_[ ii ].fd != -1 )
        {
            close( fds_[ ii ].fd );
            fds_[ ii ].fd = -1;
        }
    }
    
    fds_count_ = 1;
}

}
