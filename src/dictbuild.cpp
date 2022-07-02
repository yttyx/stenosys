/* steno Plover

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

#include "log.h"
#include "dictionary.h"

#define LOG_SOURCE "DICTB"

const char * VERSION = "0.666";

using namespace stenosys;

namespace stenosys
{

extern C_log      log;

}

/** \brief main function for dictbuild, the dictionary builder for stenosys

    Run using the configured mode

    @param[in]      argc: Number of parameters
    @param[in]      argv: Array of parameter strings
*/
int main( int argc, char *argv[] )
{
    try
    {
        log.initialise( C_log::LL_INFO, true );

        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "dictbuild version: %s", VERSION );

        C_dictionary dictionary;
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
