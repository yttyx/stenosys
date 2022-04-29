// stroke.h
#pragma once

#include <cstdint>
#include <string>
#include <memory>

using namespace stenosys;

namespace stenosys
{


class C_stroke
{

public:

    C_stroke() {}
    ~C_stroke(){}

    void
    set_steno( const std::string & steno );

    const std::string &
    get_steno();

    uint16_t
    get_seqnum();

    bool
    get_extends();

    void
    clear();

private:

    std::string      steno_;                // Steno
};

}
