// geminipr.cpp

#include <memory>
#include <stdio.h>
#include <string>

#include "dictsearch.h"
#include "log.h"
#include "miscellaneous.h"

#define LOG_SOURCE "SERCH"

using namespace stenosys;

namespace stenosys
{

extern C_log log;

C_dictionary_search::C_dictionary_search()
    : port_( -1 )
{
}

bool
C_dictionary_search::initialise( int port )
{
    tcpserver_ = std::make_unique< C_tcp_server >();

    return tcpserver_->initialise( port, "Dictionary search" );
}

bool
C_dictionary_search::start()
{
    return tcpserver_->start();
}

void
C_dictionary_search::stop()
{
   tcpserver_->stop();
}

void
C_dictionary_search::find ( const std::string & word )
{
    //TBW
}

}
