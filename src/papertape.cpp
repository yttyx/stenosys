// geminipr.cpp

#include <memory>
#include <stdio.h>
#include <string>

#include "log.h"
#include "miscellaneous.h"
#include "papertape.h"

#define LOG_SOURCE "PAPER"

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_paper_tape::C_paper_tape()
    : port_( -1 )
{
}

bool
C_paper_tape::initialise( int port )
{
    tcpserver_ = std::make_unique< C_tcp_server >();

    return tcpserver_->initialise( port, "Paper Tape" );
}

bool
C_paper_tape::start()
{
    return tcpserver_->start();
}

void
C_paper_tape::stop()
{
   tcpserver_->stop();
}

void 
C_paper_tape::write( const S_geminipr_packet & steno_packet )
{
    std::string line = C_gemini_pr::to_paper( steno_packet ) + "\r\n";

    tcpserver_->send_text( line.c_str() );
}

}
