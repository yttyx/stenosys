// geminipr.cpp

#include <assert.h>
#include <cstdint>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "geminipr.h"
#include "log.h"
#include "miscellaneous.h"

#define LOG_SOURCE "GEMPR"

using namespace stenosys;

namespace stenosys
{

extern C_log log;

uint8_t & S_geminipr_packet::operator[]( std::size_t byte_index )
{
    assert( ( byte_index >= 0 ) && ( byte_index <= BYTES_PER_STROKE ) );

    return data[ byte_index ];
}

const uint8_t & S_geminipr_packet::operator[]( std::size_t byte_index ) const
{
    assert( ( byte_index >= 0 ) && ( byte_index <= BYTES_PER_STROKE ) );

    return data[ byte_index ];
}

S_geminipr_packet *
S_geminipr_packet::get()
{
    return this;
}

S_geminipr_packet *
C_gemini_pr::encode( const std::string & stroke )
{
    //TODO (for unit test)
    return new S_geminipr_packet();
}

std::string
C_gemini_pr::decode( const S_geminipr_packet & packet )
{
    //log_writeln_fmt( C_log::LL_VERBOSE_3, LOG_SOURCE, "Stroke (binary): %02x %02x %02x %02x %02x %02x",
    //                                      packet[ 0 ], packet[ 1 ], packet[ 2 ], packet[ 3 ], packet[ 4 ], packet[ 5 ] );

    // Build up keystrokes from the bits set in the received data
    std::string stroke_lhs;
    std::string stroke_rhs;

    for ( unsigned int byte_index = 0; byte_index < BYTES_PER_STROKE; byte_index++ )
    {
        uint8_t byte = packet[ byte_index ];
        
        for ( unsigned int bit = 1; bit <= 7; bit++ )
        {
            if ( ( byte << bit ) & 0x80 )
            {
                const char key = steno_key_chart[ ( byte_index * 7 ) + bit - 1 ];

                if ( byte_index <= 2 )
                {
                    // LHS 'S' and '*' keys are effectively one ganged key, so suppress a second instance
                    if ( ( key == 'S' ) || ( key == '*' ) )
                    {
                        if ( stroke_lhs.find( key ) == std::string::npos )
                        {
                            stroke_lhs+= key;
                        }
                    }
                    else
                    {
                        stroke_lhs += key;
                    }
                }
                else
                {
                    // RHS keys
                    stroke_rhs += key;
                }
            }
        }
    }

    if ( ! suppress_hyphen( stroke_lhs, stroke_rhs ) )
    {
        stroke_lhs += "-";
    }

    return stroke_lhs + stroke_rhs;
}


std::string
C_gemini_pr::to_paper( const S_geminipr_packet & packet )
{
    std::string paper;

    for ( unsigned int byte_index = 0; byte_index < BYTES_PER_STROKE; byte_index++ )
    {
        uint8_t byte = packet[ byte_index ];
        
        for ( unsigned int bit = 1; bit <= 7; bit++ )
        {
            if ( ( byte << bit ) & 0x80 )
            {
                paper += steno_key_chart[ ( byte_index * 7 ) + bit - 1 ];
            }
            else
            {
                paper += ' ';
            }
        }
    }
    return paper;
}

bool
C_gemini_pr::suppress_hyphen( const std::string & lhs, const std::string & rhs )
{
    if ( lhs.find_first_of( "AO*", 0 ) != std::string::npos )
    {
        return true;
    }

    if ( rhs.find_first_of( "EU", 0 ) != std::string::npos )
    {
        return true;
    }

    if ( rhs.length() == 0 )
    {
        return true;
    }

    return false;
}

// When running the Steno layer in QMK, all '*' keys come through as left-hand keys
const char C_gemini_pr::steno_key_chart[] =
{
    '?', '#', '#', '#', '#', '#', '#'   // Left hand keys
,   'S', 'S', 'T', 'K', 'P', 'W', 'H'
,   'R', 'A', 'O', '*', '*', '?', '?'
,   '?', '*', '*', 'E', 'U', 'F', 'R'   // Right hand keys
,   'P', 'B', 'L', 'G', 'T', 'S', 'D'
,   '#', '#', '#', '#', '#', '#', 'Z'
};

}
