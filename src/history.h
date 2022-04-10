// history.h
#pragma once

#include <string>
#include <memory>

#include "dictionary.h"
#include "formatter.h"

using namespace stenosys;

namespace stenosys
{

const uint8_t STROKE_BUFFER_MAX = 10U;

class C_stroke
{

public:

    C_stroke() {}
    ~C_stroke(){}
    
    void
    set_next( C_stroke * next );
    
    void
    set_prev( C_stroke * prev );

    void
    clear();

private:

    C_stroke *       prev_;                 // Links to previous and next strokes
    C_stroke *       next_;                 //


    std::string      steno_;                // Steno
    bool             found_;                // Steno entry was found in dictionary
    
    const char *     translation_;          // Steno translation
    const uint16_t * flags_;                // Formatting flags

    uint16_t         stroke_seqnum_;        // The position of this stroke in a multi-stoke word
    
    bool             best_match_;           // True if the stroke represents the best match
    bool             superceded_;           // The output from this stroke has been superceded by a later stroke
                                            // with a longer steno match    
};

class C_stroke_history
{

public:

    C_stroke_history();
    ~C_stroke_history(){}

    bool
    initialise( const std::string dictionary_path );

    bool
    lookup( const std::string & steno, std::string & output );

    void 
    find_best_match( const std::string & steno, const std::string & steno_key, std::string & translation );

private:

private:

    C_stroke strokes_[ STROKE_BUFFER_MAX ];

    std::unique_ptr< C_dictionary > dictionary_;

};

}
