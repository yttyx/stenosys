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
#include "keyboard.h"
#include "keyevent.h"
#include "log.h"
#include "miscellaneous.h"
#include "papertape.h"
#include "promicrooutput.h"
#include "stenokeyboard.h"
#include "stenosys.h"
#include "strokefeed.h"
#include "translator.h"

//TEMP test
#include "tcpserver.h"

#ifdef X11
#include "x11output.h"
#endif

#define LOG_SOURCE "STSYS"

using namespace stenosys;

namespace stenosys
{

extern C_config   cfg;
extern C_log      log;

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
    if ( ! cfg.read( argc, argv ) )
    {
        fprintf( stderr, "Failed to read stenosys configuration" );
        return;
    }

    log.initialise( ( C_log::eLogLevel ) cfg.c().display_verbosity, cfg.c().display_datetime );

    //TEMP test
    C_tcp_server tcp_server;

    log_writeln( C_log::LL_INFO, LOG_SOURCE, "Before TCP initialise worked" );

    if ( tcp_server.initialise( 6666, "Test TCP server" ) )
    {
        log_writeln( C_log::LL_INFO, LOG_SOURCE, "TCP initialise worked" );
    }
    else
    {
        log_writeln( C_log::LL_INFO, LOG_SOURCE, "TCP initialise failed" );
    }

    tcp_server.start();

    std::string line;

    while ( tcp_server.running() )
    {
        if ( tcp_server.get_line( line, 128 ) )
        {
            fprintf( stdout, "%s", line.c_str() );
            fflush( stdout );
        }

        if ( line.find( 'q' ) != std::string::npos )
        {
            break;
        }

        //delay( 1 );
    }

    tcp_server.stop();
    
    exit( 0 );
    //TEMP end

    C_keyboard kbd;

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Stenosys version: %s", VERSION );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Stenosys date   : %s", __DATE__ );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Dictionary path : %s", cfg.c().file_dict.c_str() );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Raw device      : %s", cfg.c().device_raw.c_str() );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Steno device    : %s", cfg.c().device_steno.c_str() );
#ifndef X11
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Output device   : %s", cfg.c().device_output.c_str() );
#endif

    // Allow time for the key up event to occur when enter was pressed to execute this program
    delay( 1000 );
    
    bool worked = true;
    
    // If X11 is specified, use the x11 output mode; otherwise send key event data via a serial
    // port to an externally-connected Pro Micro.
#ifdef X11
#pragma message( "Building for X11" )
    std::unique_ptr< C_outputter > outputter = std::make_unique< C_x11_output >();
    
    worked = worked && outputter->initialise();
#else
#pragma message( "Building for Pro Micro" )
    std::unique_ptr< C_outputter > outputter = std::make_unique< C_pro_micro_output >();
    
    worked = worked && outputter->initialise( cfg.c().device_output );
#endif

    //TEMP
    //for ( int ii = 0; ii < 10; ii++ )
    //{
        //outputter->test();
        //delay( 1000 );
    //}

    //outputter->test();
    //TEMP end

    if ( ! worked )
    {
        log_writeln( C_log::LL_INFO, LOG_SOURCE, "Outputter initialisation failed" );
    }

    C_steno_keyboard steno_keyboard;        // Steno/raw input from the steno keyboard
    C_stroke_feed    stroke_feed;
    C_translator     translator( AT_ROMAN );
    C_paper_tape     paper_tape;

    worked = worked && steno_keyboard.initialise( cfg.c().device_raw, cfg.c().device_steno );

    //worked = worked && stroke_feed.initialise( "./stenotext/alice.steno" );    //TEST
    worked = worked && stroke_feed.initialise( "./stenotext/test.steno" );       //TEST
    worked = worked && steno_keyboard.start();
    worked = worked && translator.initialise( cfg.c().file_dict );
    worked = worked && paper_tape.initialise( 6666 );
    worked = worked && paper_tape.start();

    if ( worked )
    {
        log_writeln( C_log::LL_INFO, LOG_SOURCE, "Ready" );
            
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
                translator.translate( packet, translation );

                if ( translation.length() > 0 )
                {
                    outputter->send( translation );
                }

                if ( translator.paper_tape() )
                {
                    paper_tape.write( packet );
                }
            }

            // Key event input
            if ( steno_keyboard.read( key_event, scancode ) )
            {
                outputter->send( key_event, scancode );
            }

            delay( 1 );
        }

        log_writeln( C_log::LL_INFO, LOG_SOURCE, "Closing down" );
        
        paper_tape.stop();
        steno_keyboard.stop();

        log_writeln( C_log::LL_INFO, LOG_SOURCE, "Closed down" );
    }
    else
    {
        log_writeln( C_log::LL_ERROR, LOG_SOURCE, "Initialisation error" );
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
