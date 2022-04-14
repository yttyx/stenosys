// stroke.h
#pragma once

#include <string>
#include <memory>

#include "dictionary.h"
#include "stroke.h"

using namespace stenosys;

namespace stenosys
{

#define STROKE_BUFFER_MAX 12


enum space_type { SP_NONE, SP_BEFORE, SP_AFTER };

class C_stroke
{

public:

    C_stroke() {}
    ~C_stroke(){}

    static 
    bool initialise();

    void
    find_best_match( std::unique_ptr< C_dictionary > dictionary
                   , const std::string &             steno
                   , const std::string &             steno_key
                   , std::string &                   translation );

    static void
    clear_all();

    static std::string
    dump();
    
//    static void
//    set_curr( C_stroke * next );
private:

#if 0
    void
    set_next( C_stroke * next );
    
    void
    set_prev( C_stroke * prev );

    C_stroke * 
    next();

#endif

    void
    clear( C_stroke * stroke );

    std::string 
    undo( C_stroke ** stroke, space_type space_mode );

private:

    C_stroke *       prev_;                 // Links to previous and next strokes
    C_stroke *       next_;                 //

    std::string      steno_;                // Steno
    bool             found_;                // Steno entry was found in dictionary
    
    std::string      translation_;          // Steno translation
    uint16_t         flags_;                // Formatting flags

    uint16_t         stroke_seqnum_;        // The position of this stroke in a multi-stroke word
    
    bool             best_match_;           // True if the stroke represents the best match
    bool             superceded_;           // The output from this stroke has been superceded by a later stroke
                                            // with a longer steno match    

    static C_stroke * curr_;

};

}
