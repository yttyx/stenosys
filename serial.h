#pragma once

#include <string>

namespace stenosys
{

class C_serial
{
public:

    C_serial();
    ~C_serial();

    bool
    initialise( const std::string & device );

    void
    send( std::string & str );

    bool
    send( __u16 key_code );

    void
    stop();

protected:

    bool
    set_interface_attributes( int handle, int baudrate );

private:
    
   int  handle_;

};

}


