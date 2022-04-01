// geminipr.cpp

//#include <sys/ioctl.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <termios.h>
//include <unistd.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
//#include <stdlib.h>

#include "geminipr.h"
#include "log.h"
#include "miscellaneous.h"

#define LOG_SOURCE "GEMPR"

using namespace stenosys;

namespace stenosys
{

extern C_log log;


void
S_geminipr_packet::put( int index, uint8_t b )
{
    if ( index < BYTES_PER_STROKE )
    {
        data[ index ] = b;
    }
}

S_geminipr_packet &
S_geminipr_packet::get()
{
    return data;
}

std::string
C_gemini_pr::convert_stroke( S_geminipr_packet packet )
{
    //log_writeln_fmt( C_log::LL_VERBOSE_3, LOG_SOURCE, "Stroke (binary): %02x %02x %02x %02x %02x %02x",
    //                                      packet[ 0 ], packet[ 1 ], packet[ 2 ], packet[ 3 ], packet[ 4 ], packet[ 5 ] );

    // Build up keystrokes from the bits set in the received data
    std::string stroke_lhs;
    std::string stroke_rhs;

    for ( unsigned int byte = 0; byte < BYTES_PER_STROKE; byte++ )
    {
        unsigned char b = packet[ byte ];
        
        for ( unsigned int bit = 1; bit <= 7; bit++ )
        {
            if ( ( b << bit ) & 0x80 )
            {
                const char key = steno_key_chart[ ( byte * 7 ) + bit - 1 ];

                //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "key: %c", key );

                if ( byte <= 2 )
                {
                    // LHS 'S' and '*' keys are effectively one ganged key, so suppress a second instance
                    if ( ( key == 'S' ) || ( key == '*' ) )
                    {
                        add_if_unique( key, stroke_lhs );
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

    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "stroke_lhs: %s,  stroke_rhs: %s"
    //                                           , stroke_lhs.c_str()
    //                                           , stroke_rhs.c_str() );

    if ( ! suppress_hyphen( stroke_lhs, stroke_rhs ) )
    {
        stroke_lhs += "-";
    }

    return stroke_lhs + stroke_rhs;
}

void
C_gemini_pr::add_if_unique( char key, std::string & stroke )
{
    if ( stroke.find( key ) == std::string::npos )
    {
        stroke += key;
    }
}

// "...and if they are not unique then IMPLICIT_HYPHEN_KEYS should be set to the letters that repeat?
//  Benoit Pierre: no
//  Benoit Pierre: the center group of unique letters
//  Benoit Pierre: if one of them is present, there's no need for the hyphen"
// nsjw: IMPLICIT_HYPHEN_KEYS is defined in Plover
//       Review the logic below to see if an improvement can be made

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
