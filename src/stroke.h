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

    std::string &
    translation();

    void
    shavian( const std::string & shavian );

    std::string &
    shavian();
    
    void 
    flags( uint16_t flags );
    
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
    
    std::string      translation_;          // Steno translation
    std::string      shavian_;              // Steno translation
    uint16_t         flags_;                // Formatting flags

    uint16_t         seqnum_;               // The position of this stroke in a multi-stroke word
};

}
