#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits.h>
#include <memory>
#include <regex>
#include <unordered_map>
#include <utility>

#include "keyboard.h"
#include "miscellaneous.h"
#include "shavian_dictionary.h"
#include "log.h"
#include "textfile.h"

#define LOG_SOURCE "DICT "

namespace stenosys
{

extern C_log log;


const char * REGEX_SHAVIAN_DICTIONARY = "^(.*?)\t(.*?)\t.*$";  // tab separated variables (first two fields)

C_shavian_dictionary::C_shavian_dictionary()
    : initialised_( false )
{
    dictionary_ = std::make_unique< std::unordered_map< std::string, std::string > >();
}

C_shavian_dictionary::~C_shavian_dictionary()
{
}

// Read in tab-delimited format dictionary
bool
C_shavian_dictionary::read( const std::string & path )
{
    try
    {
        if ( C_text_file::read( path ) )
        {
            uint32_t entry_count    = 0;
            uint32_t non_data_count = 0;
            
            log_writeln( C_log::LL_INFO, LOG_SOURCE, "Reading dictionary" );
            
            std::string line;
            std::string latin_prev;
    
            while ( get_line( line ) )
            {
                std::string latin;      // Latin alphabet text
                std::string shavian;    // Shavian text

                // Check for valid entry
                if ( parse_line( line, REGEX_SHAVIAN_DICTIONARY, latin, shavian ) )
                {
                    // Maintain count of entries that should be added, as a crosscheck. Unsorted maps have
                    // unique keys so if a key already exists, calling insert() will have no effect on the map.
                    if ( latin != latin_prev )
                    {
                        entry_count++;
                        latin_prev = latin;
                    }

                    dictionary_->insert( std::make_pair( latin, shavian ) );
                }
                else
                {
                    non_data_count++;
                }
            }

            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%u entries loaded", entry_count );
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%u non-data", non_data_count );

            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%u dictionary entries", dictionary_->size() );
        }

        return true;
    }
    catch ( std::exception & ex )
    {
        error_message_ = ex.what();
    }
    catch( ... )
    {
    }

    return false;
}

bool
C_shavian_dictionary::lookup( const std::string & latin, std::string & shavian )
{
    auto result = dictionary_->find( latin );

    if ( result == dictionary_->end() )
    {
        return false;
    }
    else
    {
        shavian = result->second;
    }

    return true;
}

bool
C_shavian_dictionary::parse_line( const std::string & line, const char * regex, std::string & param1, std::string & param2 )
{
    std::regex regex_entry( regex );

    std::smatch matches;

    if ( std::regex_search( line, matches, regex_entry ) )
    {
        std::ssub_match match1 = matches[ 1 ];
        std::ssub_match match2 = matches[ 2 ];

        param1 = match1.str();
        param2 = match2.str();

        return true;
    }

    return false;
}

}
