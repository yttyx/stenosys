// history.h
#pragma once

#include <string>
#include <memory>

#include "dictionary.h"

using namespace stenosys;

namespace stenosys
{

enum space_type { SP_NONE, SP_BEFORE, SP_AFTER };

const uint8_t STROKE_BUFFER_MAX = 10U;


// TODO Move C_stroke to stroke.cpp
class C_stroke
{

public:

    C_stroke() {}
    ~C_stroke(){}
    
    void
    set_next( C_stroke * next );
    
    void
    set_prev( C_stroke * prev );

    C_stroke * 
    next();

    void
    clear();

    void
    find_best_match( std::unique_ptr< C_dictionary > dictionary
                   , const std::string &             steno
                   , const std::string &             steno_key
                   , std::string &                   translation );

    std::string 
    undo( C_stroke ** stroke, space_type space_mode );

private:

    C_stroke *       prev_;                 // Links to previous and next strokes
    C_stroke *       next_;                 //


    std::string      steno_;                // Steno
    bool             found_;                // Steno entry was found in dictionary
    
    std::string      translation_;          // Steno translation
    const uint16_t   flags_;                // Formatting flags

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

private:

    std::string
    undo();

    std::string
    dump_stroke_buffer();

    void
    clear_all_strokes();

private:

    C_stroke strokes_[ STROKE_BUFFER_MAX ];
    C_stroke * stroke_curr_;

    std::unique_ptr< C_dictionary > dictionary_;

};

}
