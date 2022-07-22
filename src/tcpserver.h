#pragma once

#include <string>

namespace stenosys
{


class C_tcp_server
{
public:

    C_tcp_server();
    ~C_tcp_server();


    bool
    initialise( int port );

    bool 
    send_text( const std::string & message );

private:

    void
    cleanup();

private:

    int socket_;
    int client_;
    int port_;

    std::string errfn_;
    int         errno_;
};

}
