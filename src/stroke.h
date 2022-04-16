// stroke.h
#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include "dictionary.h"
#include "stroke.h"

using namespace stenosys;

namespace stenosys
{

#define STROKE_BUFFER_MAX 12
#define LOOKBACK_MAX      6

class C_stroke
{

public:

    C_stroke() {}
    ~C_stroke(){}

    void
    set_next( C_stroke * );
    
    void
    set_prev( C_stroke * );

    C_stroke *
    get_next();

    C_stroke *
    get_prev();

    const std::string &
    get_steno();
    
    const std::string &
    get_translation();

    uint16_t
    get_flags();

    uint16_t
    get_seqnum();

    bool
    get_superceded();

    void
    clear();

private:


private:

    C_stroke *       prev_;                 // Links to previous and next strokes
    C_stroke *       next_;                 //

    std::string      steno_;                // Steno
    bool             found_;                // Steno entry was found in dictionary
    
    std::string      translation_;          // Steno translation
    uint16_t         flags_;                // Formatting flags

    uint16_t         seqnum_;               // The position of this stroke in a multi-stroke word
    
    bool             best_match_;           // True if the stroke represents the best match
    bool             superceded_;           // The output from this stroke has been superceded by a later stroke
                                            // with a longer steno match    
};

class C_strokes
{

public:

    C_strokes() {}
    ~C_strokes(){}

    bool
    initialise();

    void
    clear_all();

    std::string
    dump();
    
private:

    static std::string
    ctrl_to_text( const std::string & text );

private:

    C_stroke * curr_;
};

}
