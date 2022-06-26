#pragma once

#include "keyevent.h"
#include <string>

namespace stenosys
{

enum formatter_mode { FM_NONE, FM_CONSOLE, FM_ARDUINO };

class C_outputter
{

public:

    C_outputter(){}
    virtual ~C_outputter(){}

    virtual bool
    initialise() = 0;

    virtual void
    send( const std::string & text ) = 0;

    virtual void
    send( key_event_t key_event, uint8_t scancode ) = 0;
    
    virtual void
    send( uint16_t key_code ) = 0;

    virtual void
    toggle_shavian() = 0;
    
    virtual void
    test() = 0;

    virtual void
    stop() = 0;

private:

    //virtual std::string
    //format( const std::string & text ) = 0;

private:

    formatter_mode mode_;
};

}
