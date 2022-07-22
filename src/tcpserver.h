#pragma once

//#include <memory>
//#include <stdio.h>


//using namespace stenosys;

#include <cstdint>
namespace stenosys
{


class C_tcp_server
{
public:

    C_tcp_server();
    
    ~C_tcp_server() {}


    bool
    initialise( int port );

protected:


private:

    int socket_;
    int client_;
    int port_;

};

}
