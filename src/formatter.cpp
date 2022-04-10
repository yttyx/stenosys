
#include <algorithm>

#include <cstdint>
#include <cstring>
#include <iostream>

//#include "arduino_keyboard_modifiers.h"
#include "formatter.h"

using namespace stenosys;

namespace stenosys
{

std::string
C_formatter::format( const std::string & text )
{
    if ( mode_ == FM_NONE )
    {
        return text;
    }

	std::string from  = "\b";
	std::string to;

    switch( mode_ )
    {
        case FM_ARDUINO: to = "\xB2";  break;       // \xB2 is ARDUINO_KEY_BACKSPACE (arduino_keyboard_modifiers.h)
        case FM_CONSOLE: to = "\b \b"; break;
        case FM_NONE:                  break;       // Won't get here but suppress compiler warning on not handling FM_NONE
    }

    std::string output         = text;
    std::string::size_type pos = 0;

    while ( ( pos = output.find( from, pos ) ) != std::string::npos )
    {
        output.replace( pos, from.size(), to );
        pos += to.size();
    }

    return output;
}

}