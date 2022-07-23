#pragma once

//#include <string>
//#include <stdint.h>

#include "geminipr.h"
#include "tcpserver.h"

namespace stenosys
{

class C_paper_tape
{

public:
    
    C_paper_tape();
    ~C_paper_tape() {}

    bool
    initialise( int port );

    bool
    start();

    void
    stop();

    void
    write( const S_geminipr_packet steno_packet );

private:
    

private:

    bool port_;

    std::unique_ptr< C_tcp_server > tcpserver_;
};

}
