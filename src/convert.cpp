

#include <X11/Intrinsic.h>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>
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
//extern C_keyboard kbd;



//TEMP: Shavian conversion
//{
    //std::unique_ptr< C_convert > conv = std::make_unique< C_convert >();

    //conv->convert( cfg.c().file_dict
                 //, "./dictionary/kingsleyreadlexicon.tsv"
                 //, "./dictionary/yttyx-dict.csv" );
//}
//exit( 0 );
//TEMP: end



// TODO: Read through steno dictionary sequentially - putting the entries into an unsorted map
//       will cause major dictionary churn.
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
        STENO_ENTRY_2 steno_entry_2;

        bool display = true;

        if ( steno_dictionary_->get_first( steno_entry_2 ) )
        {
            std::ofstream file_stream( output_dict, std::ios::out | std::ios::binary );

            if ( file_stream.is_open() )
            {
                do
                {
                    //if ( kbd.abort() )
                    //{
                        //break;
                    //}

                    std::string shavian_key = steno_entry_2.text;

                    // Convert text to lowercase before Shavian lookup
                    std::transform( shavian_key.begin(), shavian_key.end(), shavian_key.begin(), ::tolower );
                   
                    std::string shavian;

                    shavian_dictionary_->lookup( shavian_key, shavian );

                    if ( display )
                    {
                        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "\"%s\",\"%s\",\"%s\""
                                                                   //, steno_entry_2.steno.c_str()
                                                                   //, steno_entry_2.text.c_str()
                                                                   //, shavian.c_str() );
                        
                        file_stream << "\"" << steno_entry_2.steno.c_str() << "\",";
                        file_stream << "\"" << steno_entry_2.text.c_str() << "\",";
                        file_stream << "\"" << shavian.c_str() << "\"\n";

                        //file_stream.flush();
                        //delay( 500 );
                    }
                } while ( steno_dictionary_->get_next( steno_entry_2 ) );
            
                file_stream.close();
            }

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
