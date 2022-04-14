// history.h
#pragma once

#include <string>
#include <memory>

#include "dictionary.h"
#include "stroke.h"

using namespace stenosys;

namespace stenosys
{

const uint8_t STROKE_BUFFER_MAX = 10U;


class C_stroke_history
{

public:

    C_stroke_history();
    ~C_stroke_history(){}

    bool
    initialise();

    bool
    lookup( const std::string & steno, std::string & output, uint16_t & flags, uint16_t & flags_prev );

private:

    std::string
    undo();

    std::string
    dump_stroke_buffer();

private:

    C_stroke strokes_[ STROKE_BUFFER_MAX ];
    C_stroke * stroke_curr_;


};

}
