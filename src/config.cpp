// config.cpp

#include <assert.h>
#include <cstddef>
#include <math.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "log.h"
#include "miscellaneous.h"

#define LOG_SOURCE "CONFG"

using namespace  stenosys;

namespace stenosys
{

C_config cfg;
extern C_log log;

C_config::C_config()
{
}

C_config::~C_config()
{
}

bool
C_config::read( int argc, char *argv[] )
{
    if ( ! check_params( argc, argv ) )
    {
        return false;
    }

    // The stenosys configuration is stored under the user's home directory
    //
    // If directory ~/.stenosys does not exist, create it

    if ( ! directory_exists( CONFIG_DIR ) )
    {
        create_directory( CONFIG_DIR );
    }

    //
    // If file ~/.stenosys/config does not exist, create it and write a set
    // of default parameters to it.
    //

    if ( ! file_exists( CONFIG_PATH ) )
    {
        create_default_config( CONFIG_PATH );
    }

    // Open config file. If open fails, log a message and exit (return false)
    
    if ( ! read_config( CONFIG_PATH ) )
    {
        return false;
    }

    return true;
}

bool
C_config::read_config( const std::string & config_path )
{
    // Read in whole file
    if ( ! C_text_file::read( CONFIG_FILE ) )
    {
        log_writeln( C_log::LL_ERROR, LOG_SOURCE, "Error loading configuration file" );
        return false;
    }

    // For each line in the file, parse it using a regexes
    // - Detect comment lines
    //   - ignore
    // - Detect parameter lines
    //   - Extract parameter & value from line
    //   - Check for valid parameter; set cfg data member if so
    // - For other lines
    //   - Ignore/report on invalid lines

    return true;
}

void
C_config::create_default_config( const std::string & config_path )
{
    FILE * output_stream = fopen( config_path.c_str(), "w" );

    if ( output_stream != nullptr )
    {
        fprintf( output_stream, OPT_DISPLAY_VERBOSITY "=%s", DEF_DISPLAY_VERBOSITY );
        fprintf( output_stream, OPT_DISPLAY_DATETIME  "=%s", DEF_DISPLAY_DATETIME  );
        fprintf( output_stream, OPT_FILE_STENOFILE    "=%s", DEF_FILE_STENOFILE    );
        fprintf( output_stream, OPT_DICTIONARY        "=%s", DEF_DICTIONARY        );
        fprintf( output_stream, OPT_RAW_DEVICE        "=%s", DEF_RAW_DEVICE        );
        fprintf( output_stream, OPT_STENO_DEVICE      "=%s", DEF_STENO_DEVICE      );
        fprintf( output_stream, OPT_SERIAL_OUTPUT     "=%s", DEF_SERIAL_OUTPUT     );

        fclose( output_stream );
    }
    else
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Error accessing %s", config_path.c_str() );
    }
}

bool
C_config::check_params( int argc, char *argv[] )
{
    if ( argc > 1 )
    {
        // Program takes no parameters; show usage if any are supplied

        log_writeln( C_log::LL_INFO, LOG_SOURCE, "stenosys - stenographic utility" );
        log_writeln( C_log::LL_INFO, LOG_SOURCE, "  Its configuration file is stored at " CONFIG_FILE );
        log_writeln( C_log::LL_INFO, LOG_SOURCE, "" );

        return false;
    }

    return true;
}

}
