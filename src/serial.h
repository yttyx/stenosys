#pragma once

#include <string>
#include <sys/types.h>

namespace stenosys
{

class C_serial
{
public:

    C_serial();
    ~C_serial();

    bool
    initialise( const std::string & device );

    bool
    send( uint8_t ch );

protected:

    bool
    set_interface_attributes( int handle, int baudrate );

private:
    
    int         handle_;
    std::string device_;
};

}


