// stroke.h
#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include "dictionary.h"
#include "history.h"
#include "stroke.h"

using namespace stenosys;

namespace stenosys
{

#define STROKE_BUFFER_MAX 12
#define LOOKBACK_MAX      6

class C_strokes
{

public:

    C_strokes( C_dictionary & dictionary );
    ~C_strokes();

    bool
    initialise();

    void
    find_best_match( const std::string &               steno
                   , std::string &                     text 
                   , uint16_t &                        flags
                   , uint16_t &                        flags_prev 
                   , bool &                            extends );

    void
    undo();

    void
    translation( const std::string translation );
    
    std::string &
    translation();

    std::string
    previous_translation();

    bool
    extends();

    void
    clear();

    void 
    dump();
    
private:

    void
    find_best_match( uint16_t                          level
                   , const std::string &               steno_key
                   , std::string &                     text
                   , C_stroke * &                      best_match );

    static std::string
    ctrl_to_text( const std::string & text );

private:

    C_dictionary & dictionary_;

    std::unique_ptr< C_history< C_stroke, 10 > > history_;
};

}
