/* stenosys.cpp: stenosys is a stenography utility, a cut-down version of Plover

Copyright (C) 2020  yttyx

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/*! \file stenosys.cpp
    \brief Top-level code
 */
#include <linux/types.h>

#include <string>
#include <unistd.h>

#include "config.h"
#include "device.h"
#include "dictionary.h"
#include "geminipr.h"
#include "keyboard.h"
#include "log.h"
#include "miscellaneous.h"
#include "stenokeyboard.h"
#include "stenosys.h"

#define LOG_SOURCE "SITM "

using namespace stenosys;

namespace stenosys
{

extern C_config   cfg;
extern C_log      log;
extern C_keyboard kbd;

const char * VERSION = "0.10";

C_stenosys::C_stenosys()
{
}

C_stenosys::~C_stenosys()
{
}

/** \brief main function

    Run using the configured mode

    @param[in]      argc: Number of parameters
    @param[in]      argv: Array of parameter strings
*/
void
C_stenosys::run( int argc, char *argv[] )
{
    if ( cfg.read( argc, argv ) )
    {
        // space_type sm = cfg.c().space_after ? SP_AFTER : SP_BEFORE;

        //TEMP test dictionary building
        C_dictionary dictionary;


        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "cfg.c().file_dict: %s", cfg.c().file_dict.c_str() );


        dictionary.build( cfg.c().file_dict, "./dict-output.txt" );

        //TEMP:END


        C_steno_keyboard steno_keyboard;                        // Steno/raw x input from the steno ;keyboard
        // C_steno_translator  translator( sm, FM_ARDUINO );    // Steno to English convertor
        // C_serial            serial;                          // Serial output to the Pro Micro
        // C_stroke_feed       stroke_feed;                     // Steno stroke feed for regression testing

        //log.initialise( cfg.c().display_verbosity, cfg.c().display_datetime );
 
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "stenosys version %s, date %s", VERSION, __DATE__ );

        //if ( stroke_feed.initialise( cfg.c().file_steno ) )
        //{
        //    // Allow time to move to a blank Notepad page before starting the steno test
        //    delay( 3000 );
        //}

        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "raw device  :%s", cfg.c().device_raw.c_str() );
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "steno device:%s", cfg.c().device_steno.c_str() );

        bool worked = true;

        worked = worked && steno_keyboard.initialise( cfg.c().device_raw, cfg.c().device_steno );
        worked = worked && steno_keyboard.start();
        //worked = worked && serial.initialise( cfg.c().device_output ); 

        if ( worked )
        {
            log_writeln( C_log::LL_INFO, LOG_SOURCE, "Ready" );
        
            while ( ! kbd.abort() )
            {
                __u16       key_code;
                std::string stroke;

                // Get steno input from either a steno test file or from the keyboard.
                // The steno test file has precedence. Input will be from the keyboard only
                // once all steno test file strokes have been consumed, .
                //if ( stroke_feed.read( stroke ) || steno.read( stroke ) )
                
                // Get steno input from the C_keyboard
                
                S_geminipr_packet packet;
                
                if ( steno_keyboard.read( packet ) )
                {
                    log_writeln( C_log::LL_INFO, LOG_SOURCE, "Got steno packet" );

                    std::string steno_chord = C_gemini_pr::decode( packet );

                    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "steno: %s",steno_chord.c_str() );
#if 0                
                    std::string translation;

                    if ( translator.translate( stroke, translation ) )
                    {
                        serial.send( translation );
                    }
#endif
                }
                else if ( steno_keyboard.read( key_code ) )
                {
                    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "raw input: %04u", key_code );
                    //serial.send( key_code );
                }

                delay( 1 );
            }

            log_writeln( C_log::LL_INFO, LOG_SOURCE, "Closing down" );
            
            //serial.stop();
            steno_keyboard.stop();

            log_writeln( C_log::LL_INFO, LOG_SOURCE, "Devices closed down" );
        }
        else
        {
            log_writeln( C_log::LL_INFO, LOG_SOURCE, "Initialisation error" );
        }
    }
}

}

/** \brief main function

    Run using the configured mode

    @param[in]      argc: Number of parameters
    @param[in]      argv: Array of parameter strings
*/
int main( int argc, char *argv[] )
{
    try
    {
        log.initialise( C_log::LL_INFO, true );
        
        C_stenosys stenosys;
        
        stenosys.run( argc, argv );
    }
    catch ( std::exception & ex )
    {
        fprintf( stdout, "Program exception: %s\n", ex.what() );
    }
    
    catch ( ... )
    {
        fprintf( stdout, "Program exception\n" );
    }
    
    fprintf( stdout, "Closed down\n" );

    return 0;
}
