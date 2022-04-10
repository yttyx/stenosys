// translator.h
#pragma once

#include <string>
#include <memory>

#include "formatter.h"

using namespace stenosys;

namespace stenosys
{

enum space_type { SP_NONE, SP_BEFORE, SP_AFTER };

const uint8_t STROKE_BUFFER_MAX = 10U;

struct C_stroke
{

public:

    C_stroke(){}
    ~C_stroke(){}

    void
    clear( C_stroke * stroke );

    C_stroke *
    find_best_match( const std::string & steno, const std::string & steno_key, std::string & translation );

private:


private:

    C_stroke   *     prev_;                 // Links to previous and next strokes
    C_stroke   *     next_;                 //

    std::string      steno_;                // Steno
    bool             found_;                // Steno entry was found in dictionary
    const char *     translation_;          // Steno translation
    const uint16_t * flags_;                // Formatting flags
    
    uint16_t         stroke_seqnum_;        // The position of this stroke in a multi-stoke word
    
    bool             best_match_;           // True if the stroke represents the best match
    bool             superceded_;           // The output from this stroke has been superceded by a later stroke
                                            // with a longer steno match    
    space_type       st;
};


class C_steno_translator
{

public:

    C_steno_translator( space_type trans_mode, formatter_mode format_mode );
    ~C_steno_translator();

    bool
    translate( const std::string & steno, std::string & output );
    
    void
    display_stroke_queue();

private:
    C_steno_translator(){}

    std::string 
    lookup();
    
    std::string 
    undo();

    std::string
    format_output( C_stroke * stroke_previous_to_best_match, uint8_t backspaces, C_stroke * current_stroke );

    std::string
    dump_stroke_buffer();

    void
    toggle_space_mode();

    void
    clear_all_strokes();

    std::string
    ctrl_to_text( const std::string & text );

private:
    
    space_type space_mode_;

    C_stroke   strokes_[ STROKE_BUFFER_MAX ];
    C_stroke * stroke_curr_;

    std::unique_ptr< C_formatter > formatter_;

    static const char   * NO_TRANSLATION;
    static const uint16_t NO_FLAGS;
    static const uint16_t DUMMY_ATTACH;
};

}
