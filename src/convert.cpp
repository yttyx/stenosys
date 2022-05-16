

#include <X11/Intrinsic.h>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "convert.h"
#include "dictionary.h"
#include "keyboard.h"
#include "miscellaneous.h"
#include "shavian_dictionary.h"
#include "log.h"
#include "utf8.h"

#define LOG_SOURCE "CNVRT"

using namespace stenosys;

namespace stenosys
{

extern C_log      log;
extern C_keyboard kbd;

bool
C_convert::convert( const std::string & steno_dict
                  , const std::string & shavian_dict
                  , const std::string & output_dict )
{
    steno_dictionary_   = std::make_unique< C_dictionary >();
    shavian_dictionary_ = std::make_unique< C_shavian_dictionary >();

    bool worked = true;

    worked = worked && steno_dictionary_->read( steno_dict );
    worked = worked && shavian_dictionary_->read( shavian_dict );

    if ( worked )
    {
        std::string steno;
        STENO_ENTRY steno_entry;

        if ( steno_dictionary_->get_first( steno, steno_entry ) )
        {
            do
            {
                if ( kbd.abort() )
                {
                    break;
                }

                std::string shavian;

                shavian_dictionary_->lookup( steno_entry.text, shavian );

                C_utf8 shavian_utf8( shavian );

                //int shavian_spaces = shavian_utf8.length();

                log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%04u  %-16.16s  %-20.20s  %s"
                                                           , steno_entry.flags
                                                           , steno.c_str()
                                                           , steno_entry.text.c_str()
                                                           , shavian.c_str() );

                delay( 1000 );

            } while ( steno_dictionary_->get_next( steno, steno_entry ) );
        }

        log_writeln( C_log::LL_INFO, LOG_SOURCE, "Done" );
    }
    else
    {
        log_writeln( C_log::LL_INFO, LOG_SOURCE, "Failed to read either steno or shavian dictionary" );
    }

    return false;
}

}
