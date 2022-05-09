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

#include <memory>
#include <string>
#include <unistd.h>

#include "config.h"
#include "device.h"
#include "geminipr.h"
#include "keyboard.h"
#include "log.h"
#include "miscellaneous.h"
#include "stenokeyboard.h"
#include "stenosys.h"
#include "dictionary.h"
#include "strokefeed.h"
#include "translator.h"
#include "x11output.h"

#define LOG_SOURCE "STSYS"

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
        log.initialise( ( C_log::eLogLevel ) cfg.c().display_verbosity, cfg.c().display_datetime );
 
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Stenosys version: %s", VERSION );
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Stenosys date   : %s", __DATE__ );
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Dictionary path : %s", cfg.c().file_dict.c_str() );
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Raw device      : %s", cfg.c().device_raw.c_str() );
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Steno device    : %s", cfg.c().device_steno.c_str() );

        std::unique_ptr< C_outputter > outputter = std::make_unique< C_x11_output>();

        C_steno_keyboard steno_keyboard;        // Steno/raw input from the steno keyboard */
        // C_serial       serial;               // Serial output to the Pro Micro
        C_stroke_feed    stroke_feed;
        C_translator     translator( cfg.c().space_after ? SP_AFTER : SP_BEFORE );
        
        bool worked = true;
    
        worked = worked && outputter->initialise();

        //TEMP
        if ( worked )
        {
            outputter->test();
        }
        exit( 0 );
        //TEMP:END

        worked = worked && steno_keyboard.initialise( cfg.c().device_raw, cfg.c().device_steno );
        worked = worked && steno_keyboard.start();
        //worked = worked && serial.initialise( cfg.c().device_output ); 
        
        worked = worked && translator.initialise( cfg.c().file_dict );
        worked = worked && stroke_feed.initialise( "./stenotext/alice.steno" );    //TEST

        if ( worked )
        {
            log_writeln( C_log::LL_INFO, LOG_SOURCE, "Ready" );
        
            while ( ! kbd.abort() )
            {
                std::string       stroke;
                std::string       steno;
                std::string       translation;
                S_geminipr_packet packet;
                __u16             key_code;

                //if ( stroke_feed.get_steno( steno ) )
                //{
                    //translator.translate( steno, translation );
                //}
                
                if ( steno_keyboard.read( packet ) )
                {
                    steno = C_gemini_pr::decode( packet );
                    
                    translator.translate( steno, translation );
                }

                if ( translation.length() > 0 )
                {
//                    log_write_fmt( C_log::LL_INFO, LOG_SOURCE, "%s", translation.c_str() );

                      outputter->send( translation );
                }

                if ( steno_keyboard.read( key_code ) )
                {
                    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "key event: %04x", key_code );
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
            log_writeln( C_log::LL_ERROR, LOG_SOURCE, "Initialisation error" );
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
