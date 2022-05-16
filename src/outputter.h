#pragma once

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
    test() = 0;

private:

    //virtual std::string
    //format( const std::string & text ) = 0;

private:

    formatter_mode mode_;
};

}