// stroke.h
#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include "dictionary.h"

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
    
    C_stroke *
    get_next();
    
    void
    set_prev( C_stroke * );

    C_stroke *
    get_prev();

    void
    set_steno( const std::string & steno );

    const std::string &
    get_steno();
    
    void
    set_translation( const std::string & translation );

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
    add( const std::string & steno );

    void
    find_best_match( std::unique_ptr< C_dictionary > & dictionary
                   , const std::string &               steno
                   , std::string &                     text 
                   , uint16_t &                        flags
                   , uint16_t &                        flags_prev );
    void
    clear();

    std::string
    dump();
    
private:

    void
    find_best_match( std::unique_ptr< C_dictionary > & dictionary
                   , uint16_t                          level
                   , C_stroke *                        stroke
                   , const std::string &               steno_key
                   , std::string &                     text
                   , C_stroke **                       best_match );

    static std::string
    ctrl_to_text( const std::string & text );

private:

    C_stroke * stroke_curr_;
};

}