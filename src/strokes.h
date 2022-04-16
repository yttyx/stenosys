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
                   , C_stroke * &                      best_match );

    static std::string
    ctrl_to_text( const std::string & text );

private:

    C_stroke * stroke_curr_;
};

}
