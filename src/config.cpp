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

#define LOG_SOURCE "CFG  "

using namespace  stenosys;

namespace stenosys
{

C_config cfg;
extern C_log log;

static S_option options[] =
{
    { OPT_DISPLAY_VERBOSITY, FLD_INTEGER, offsetof( S_config, display_verbosity ), "display.verbosity", "0",   "6"   },
    { OPT_DISPLAY_DATETIME,  FLD_BOOLEAN, offsetof( S_config, display_datetime ),  "display.datetime",  "",    ""    },
    { OPT_FILE_STENO,        FLD_STRING,  offsetof( S_config, file_steno ),        "file.steno",        "0",   "256" },
    { OPT_FILE_DICT,         FLD_STRING,  offsetof( S_config, file_dict ),         "file.dictionary",   "1",   "256" },
    { OPT_DEVICE_RAW,        FLD_STRING,  offsetof( S_config, device_raw ),        "device.raw",        "1",   "256" },
    { OPT_DEVICE_STENO,      FLD_STRING,  offsetof( S_config, device_steno ),      "device.steno",      "1",   "256" },
    { OPT_DEVICE_OUTPUT,     FLD_STRING,  offsetof( S_config, device_output ),     "device.output",     "1",   "256" },
    { OPT_SPACE_AFTER,       FLD_BOOLEAN, offsetof( S_config, space_after ),       "space_after",       "",    ""    },
    { OPT_NONE,              FLD_NONE,    0,                                       "",                  "0",   "0"   }
};

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
        usage();
        return false;
    }

    // Try to read a configuration if one was supplied, otherwise use a default configuration
    if ( got_config_file_ )
    {
        if ( ! read( config_file_ ) )
        {
            log_writeln( C_log::LL_ERROR, LOG_SOURCE, "Configuration error" );
            return false;
        }
    }

    if ( display_config_ )
    {
        display_settings();
        return false;
    }

    return true;
}

bool
C_config::read( std::string & filename )
{
    try
    {
        cfg_.readFile( filename.c_str() );
    }
    catch ( const FileIOException &fioex )
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "Error reading configuration file '%s'", filename.c_str() );
        return false;
    }
    catch ( const ParseException &pex )
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "Error in configuration file '%s' at line %d: %s", pex.getFile(), pex.getLine(), pex.getError() );
        return false;
    }

    // Read base profile
    bool worked = read_profile( "standard" );

    worked = worked && setting_checks();

    return worked;
}

bool
C_config::read_profile( const std::string & profile )
{
    // Check that profile exists

    std::string field_path;

    field_path = format_string( "stenosys.profile.%s", profile.c_str() );

    if ( ! cfg_.exists( field_path ) )
    {
        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "Profile '%s' does not exist in configuration file", profile.c_str() );
        return false;
    }

    const Setting & profile_settings = cfg_.lookup( field_path.c_str() );

    read_option_table( profile_settings, options, &config_ );

    // Set logging parameters
    log.initialise( cfg.c().display_verbosity, cfg.c().display_datetime );

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Applied profile: %s", profile.c_str() );

    return true;
}

void
C_config::read_option_table( const Setting & parent, const S_option * options, void * record )
{
    // Parse profile settings
    for ( ; options->opt != OPT_NONE; options++ )
    {
        try
        {
            switch ( options->field_type )
            {
                case FLD_STRING:
                    * ( ( std::string * ) ( ( ( char * ) record ) + options->field_offset ) ) = parent.lookup( options->field_path ).c_str();
                    break;

                case FLD_INTEGER:
                    * ( ( int * ) ( ( ( char * ) record ) + options->field_offset ) ) = parent.lookup( options->field_path );
                    break;

                case FLD_BOOLEAN:
                    * ( ( bool * ) ( ( ( char * ) record ) + options->field_offset ) ) = parent.lookup( options->field_path );
                    break;

                default:
                    break;
            }
        }
        catch ( const SettingNotFoundException & nfex )
        {
            log_writeln_fmt( C_log::LL_VERBOSE_3, LOG_SOURCE, "Missing setting in configuration file: %s", nfex.getPath() );
        }
        catch ( const SettingTypeException & nfex )
        {
            log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "Incorrect parameter type in configuration file: %s", nfex.getPath() );
        }
        catch ( ... )
        {
            log_writeln( C_log::LL_ERROR, LOG_SOURCE, "EXCEPTION" );
        }
    }
}

bool
C_config::setting_checks()
{
    return setting_checks( options, ( void * ) &config_ );
}

bool
C_config::setting_checks( const S_option * options, void * record )
{
    bool ok = true;

    for ( ; options->opt != OPT_NONE; options++ )
    {
        try
        {
            switch ( options->field_type )
            {
                case FLD_STRING:
                {
                    std::string str = * ( ( std::string * ) ( ( ( char * ) record ) + options->field_offset ) );
                    
                    if ( ( str.length() < ( size_t ) atoi( options->min ) ) || ( str.length() > ( size_t ) atoi( options->max ) ) )
                    {
                        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "Invalid length: %s - %s", options->field_path, str.c_str() );
                        ok = false;
                    }
                    break;
                }
                case FLD_INTEGER:
                {
                    int i = * ( ( int * ) ( ( ( char * ) record ) + options->field_offset ) );

                    if ( ( i < atoi( options->min ) ) || ( i > atoi( options->max ) ) )
                    {
                        log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "Value out of range: %s - %d", options->field_path, i );
                        ok = false;
                    }
                    break;
                }
                case FLD_BOOLEAN:
                {
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        catch ( const std::exception & ex )
        {
            log_writeln_fmt( C_log::LL_ERROR, LOG_SOURCE, "Exception in parameter checks: %s", ex.what() );
        }
        catch ( ... )
        {
            log_writeln( C_log::LL_ERROR, LOG_SOURCE, "Exception in parameter checks" );
        }
    }

    return ok;
}

void
C_config::display_settings()
{
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "stenosys configuration");
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "----------------------------------------------------------------------------------------");

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%-43.43s: %d", to_string( OPT_DISPLAY_VERBOSITY ), config_.display_verbosity );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%-43.43s: %s", to_string( OPT_DISPLAY_DATETIME ),  to_string( config_.display_datetime ) );
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "--");
}

const char *
C_config::to_string( bool setting )
{
    return setting ? "true" : "false";
}

std::string
C_config::to_string( eOption opt1, eOption opt2 )
{
    return format_string( "%s.%s", to_string( opt1 ), to_string( opt2 ) );
}

const char *
C_config::to_string( eOption opt )
{
    // Search each of the options tables until the text related to opt is found
    for ( S_option *op = options; op->opt != OPT_NONE; op++ )
    {
        if ( op->opt == opt )
        {
            return op->field_path;
        }
    }

    return "";
}

bool
C_config::check_params( int argc, char *argv[] )
{
    int opt;

    while ( ( opt = getopt( argc, argv, "c:dh?" ) ) != -1 )
    {
        switch ( opt )
        {
            case 'c':
                config_file_ = optarg;
                got_config_file_ = true;
                break;

            case 'd':
                display_config_ = true;
                break;

            case '?':
            case 'h':
            default:
                return false;
        }
    }

    if ( ! got_config_file_ )
    {
        return false;
    }

    return true;
}

void
C_config::usage()
{
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "stenosys [-c <configuration file>] [-d] [-h] [-?]" );
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "where:" );
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "    -d: display configuration settings" );
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "    -h: display valid options" );
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "    -?: display valid options" );
}

}
