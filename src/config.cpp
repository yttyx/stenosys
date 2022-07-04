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
        usage();
        return false;
    }

    // Try to read a configuration if one was supplied, otherwise use a default configuration
    if ( got_config_file_ )
    {
        if ( ! C_text_file::read( config_file_ ) )
        {
            log_writeln( C_log::LL_ERROR, LOG_SOURCE, "Configuration error" );
            return false;
        }
    }

    return true;
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
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "stenosys [-c <configuration file>] [-h] [-?]" );
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "where:" );
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "    -h: display valid options" );
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "    -?: display valid options" );
}

}
