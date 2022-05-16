

#include <X11/Intrinsic.h>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "convert.h"
#include "dictionary.h"
#include "miscellaneous.h"
#include "shavian_dictionary.h"
#include "log.h"

#define LOG_SOURCE "CNVRT"

using namespace stenosys;

namespace stenosys
{

extern C_log log;

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
                log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "steno:: text: %s, flags: %u"
                                                           , steno_entry.text.c_str()
                                                           , steno_entry.flags );

                std::string shavian;

                if ( shavian_dictionary_->lookup( steno_entry.text, shavian ) )
                {
                    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "    shavian: %s", shavian.c_str() );
                }
                else
                {
                    log_writeln( C_log::LL_INFO, LOG_SOURCE, "    (no shavian entry)" );
                }

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
