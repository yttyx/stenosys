#include <memory>
#include <string>

#include "log.h"
#include "miscellaneous.h"
#include "stenosysclient.h"
#include "window_layout.h"
#include "window_output.h"
#include "window_scroll.h"


using namespace stenosys;

namespace stenosys
{

extern C_log      log;

const char *VERSION = "0.10";


C_stenosys_client::C_stenosys_client()
{
}

C_stenosys_client::~C_stenosys_client()
{
}

/** \brief main function

    Run using the configured mode

    @param[in]      argc: Number of parameters
    @param[in]      argv: Array of parameter strings
*/
void
C_stenosys_client::run( int argc, char *argv[] )
{
    try
    {
        log_writeln_fmt( C_log::LL_INFO, "stenoclient version %s, date %s", VERSION, __DATE__ );

        std::unique_ptr< C_window_scroll > window_steno      = std::make_unique< C_window_scroll >( "Steno",      WIN_STENO_TOP,      WIN_STENO_LEFT,      WIN_STENO_HEIGHT,      WIN_STENO_WIDTH      );
        std::unique_ptr< C_window_scroll > window_translator = std::make_unique< C_window_scroll >( "Translator", WIN_TRANSLATOR_TOP, WIN_TRANSLATOR_LEFT, WIN_TRANSLATOR_HEIGHT, WIN_TRANSLATOR_WIDTH );
        std::unique_ptr< C_window_scroll > window_output     = std::make_unique< C_window_scroll >( "Output",     WIN_OUTPUT_TOP,     WIN_OUTPUT_LEFT,     WIN_OUTPUT_HEIGHT,     WIN_OUTPUT_WIDTH     );
        std::unique_ptr< C_window_scroll > window_log        = std::make_unique< C_window_scroll >( "Log",        WIN_LOG_TOP,        WIN_LOG_LEFT,        WIN_LOG_HEIGHT,        WIN_LOG_WIDTH        );
        
        log.initialise(  C_log::LL_INFO, true );

        if ( true ) //TODO: check that initialisation (TODO) was successful
        {
            delay( 10000 );

        }
        else
        {
            //kbd.wait_for_key();
        }
    }
    catch ( std::exception & ex )
    {
        log_writeln_fmt( C_log::LL_INFO, "Program exception: %s, press any key", ex.what() );
    }
    catch ( ... )
    {
        log_writeln( C_log::LL_INFO, "Program exception, press any key" );
    }

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
    C_stenosys_client client;

    client.run( argc, argv );

    return 0;
}
