/* stenosys - A stenography utility for linux

Copyright (C) 2022  yttyx

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
#include <cstdio>
#include <linux/types.h>

#include <memory>
#include <string>
#include <unistd.h>

#include "config.h"
#include "device.h"
#include "dictsearch.h"
#include "geminipr.h"
#include "keyboard.h"
#include "keyevent.h"
#include "log.h"
#include "miscellaneous.h"
#include "papertape.h"
#include "stenokeyboard.h"
#include "stenosys.h"
#include "strokefeed.h"
#include "translator.h"
#include "x11output.h"


using namespace stenosys;

namespace stenosys
{

extern C_config   cfg;
extern C_log      log;

const char * VERSION = "0.90";

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
    if ( ! cfg.read( argc, argv ) )
    {
        fprintf( stderr, "Failed to read stenosys configuration" );
        return;
    }

    log.initialise( ( C_log::eLogLevel ) cfg.c().display_verbosity, cfg.c().display_datetime );

    C_keyboard kbd;

    const char * device_raw = cfg.c().device_raw.c_str();
    const char * dict_path = cfg.c().file_dict.c_str();

    log_writeln_fmt( C_log::LL_INFO, "Stenosys version: %s", VERSION );
    log_writeln_fmt( C_log::LL_INFO, "Stenosys date   : %s", __DATE__ );
    log_writeln_fmt( C_log::LL_INFO, "Dictionary path : %s", strlen( dict_path ) > 0  ? dict_path  : "<none>" );
    log_writeln_fmt( C_log::LL_INFO, "Raw device      : %s", strlen( device_raw ) > 0 ? device_raw : "auto-detect" ); 
    log_writeln_fmt( C_log::LL_INFO, "Steno device    : %s", cfg.c().device_steno.c_str() );

    // Allow time for the key up event to occur when enter was pressed to execute this program
    delay( 1000 );
    
    bool worked = true;
    
    std::unique_ptr< C_x11_output> outputter = std::make_unique< C_x11_output >();
    
    worked = worked && outputter->initialise();
    
    if ( ! worked )
    {
        log_writeln( C_log::LL_INFO, "Outputter initialisation failed (is X Window system running?)" );
    }

    C_steno_keyboard    steno_keyboard;        // Steno/raw input from the steno keyboard
    //C_stroke_feed       stroke_feed;
    C_translator        translator( AT_LATIN );
    C_paper_tape        paper_tape;
    C_dictionary_search dictionary_search;

    worked = worked && steno_keyboard.initialise( cfg.c().device_raw, cfg.c().device_steno );

    //worked = worked && stroke_feed.initialise( "./stenotext/alice.steno" );    //TEST
    //worked = worked && stroke_feed.initialise( "./stenotext/test.steno" );     //TEST
    worked = worked && steno_keyboard.start();
    delay( 2000 );

    worked = worked && translator.initialise();

    worked = worked && paper_tape.initialise( 6666 );
    worked = worked && paper_tape.start();

    worked = worked && dictionary_search.initialise( 6668 );
    worked = worked && dictionary_search.start();
    delay( 2000 );
    
    if ( worked )
    {
        std::string       stroke;
        std::string       steno;
        std::string       translation;
        S_geminipr_packet packet;

        uint8_t           scancode  = 0;
        key_event_t       key_event = KEY_EV_UNKNOWN;
        
        while ( ! kbd.abort() )
        {
            // Stenographic chord input
            if ( steno_keyboard.read( packet ) )
            {
                //log_writeln( C_log::LL_ERROR, "Got steno chord" );
                
                translator.translate( packet, translation );
                
                //TEMP
                log_writeln_fmt( C_log::LL_VERBOSE_1, "translation: %s", translation.c_str() );
                
                if ( translation.length() > 0 )
                {
                    outputter->send( translation );
                }

                if ( translator.paper_tape() )
                {
                    paper_tape.write( packet );
                }
            }

            if ( steno_keyboard.acquired() )
            {
                outputter->set_keymapping(); 
                
                log_writeln( C_log::LL_INFO, "Ready" );
            }

            // Key event input
            if ( steno_keyboard.read( key_event, scancode ) )
            {
                //TEMP
                log_writeln_fmt( C_log::LL_VERBOSE_1, "key event: scancode: 0x%02x", scancode );

                outputter->send( key_event, scancode );
            }

            delay( 1 );
        }
    }
    else
    {
        log_writeln( C_log::LL_ERROR, "Initialisation error" );
    }

    log_writeln( C_log::LL_INFO, "Closing down" );

    dictionary_search.stop();
    paper_tape.stop();
    steno_keyboard.stop();

    log_writeln( C_log::LL_INFO, "Closed down" );
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
