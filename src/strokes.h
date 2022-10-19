// stroke.h
#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include "history.h"
#include "stenoflags.h"
#include "stroke.h"
#include "symbols.h"

using namespace stenosys;

namespace stenosys
{

#define STROKE_BUFFER_MAX 12
#define LOOKBACK_MAX      6

class C_strokes
{

public:

    C_strokes( C_symbols & symbols );
    ~C_strokes();

    bool
    initialise();

    void
    add_stroke( const std::string & steno
              , alphabet_type       alphabet
              , std::string &       text
              , uint16_t &          flags
              , uint16_t &          flags_prev 
              , bool &              extends );

    void
    add_stroke( const std::string & steno
              , std::string &       text
              , uint16_t &          flags
              , uint16_t &          flags_prev );
    
    void
    undo();

    bool
    lookup( const std::string & steno
          , alphabet_type       alphabet
          , std::string &       text
          , uint16_t &          flags );

    void
    translation( const std::string translation );
    
    std::string &
    translation();

    std::string
    previous_translation();

    uint16_t
    flags();

    uint16_t
    flags_prev();

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


    //TEMP
    //void
    //word_check( const std::string & word );

private:

    C_symbols    & symbols_;

    std::unique_ptr< C_history< C_stroke, 10 > > history_;
};

}
