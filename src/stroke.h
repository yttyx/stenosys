// stroke.h
#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include "dictionary.h"

using namespace stenosys;

namespace stenosys
{

class C_stroke
{

public:

    C_stroke();
    
    C_stroke( const std::string & steno );

    ~C_stroke(){}

    C_stroke &
    operator=( const C_stroke & stroke );

    void
    steno( const std::string & steno );

    const std::string &
    steno();
    
    void
    translation( const std::string & translation );

    const std::string &
    translation();

    uint16_t
    flags();

    void
    seqnum( uint16_t seqnum );
    
    uint16_t
    seqnum();

    bool
    extends();

    bool
    superceded();

    void
    clear();

private:


private:

    std::string      steno_;                // Steno
    bool             found_;                // Steno entry was found in dictionary
    
    std::string      translation_;          // Steno translation
    uint16_t         flags_;                // Formatting flags

    uint16_t         seqnum_;               // The position of this stroke in a multi-stroke word
};

}
