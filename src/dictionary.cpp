#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits.h>
#include <memory>
#include <regex>
#include <unordered_map>
#include <utility>

#include "cmdparser.h"
#include "dictionary.h"
#include "log.h"
#include "miscellaneous.h"
#include "stenoflags.h"
#include "textfile.h"
#include "utf8.h"

#define LOG_SOURCE "DICT "

namespace stenosys
{

extern C_log log;

const char * REGEX_DICTIONARY = "^(.*?)\t(.*?)\t(.*?)$";

C_dictionary::C_dictionary()
    : initialised_( false )
{
    parser_     = std::make_unique< C_cmd_parser >();
    dictionary_ = std::make_unique< std::unordered_map< std::string, STENO_ENTRY > >();

    //TODO Remove dict_vector_: it was used when merging the Shavian dictionary into the base dictionary
    dict_vector_ = std::make_unique< std::vector< STENO_ENTRY_2 > >();
}

C_dictionary::~C_dictionary()
{
}

// Read in JSON-format dictionary (Plover format) into an array of dictionary entries
bool
C_dictionary::read( const std::string & path )
{
    try
    {
        if ( C_text_file::read( path ) )
        {
            uint32_t entry_count    = 0;
            uint32_t non_data_count = 0;
            
            log_writeln( C_log::LL_INFO, LOG_SOURCE, "Reading dictionary" );
            
            std::string line;
    
            while ( get_line( line ) )
            {
                std::string steno;
                std::string latin;      // Latin alphabet
                std::string shavian;    // Shavian

                uint16_t latin_flags   = 0;
                uint16_t shavian_flags = 0;

                // Check for valid CSV entry
                if ( parse_line( line, REGEX_DICTIONARY, steno, latin, shavian ) )
                {
                    std::string parsed_latin;
                    std::string parsed_shavian;

                    // Parse the dictionary text for a subset of Plover commands and set steno flags
                    bool latin_ok   = parser_->parse( latin, parsed_latin, latin_flags );
                    bool shavian_ok = parser_->parse( shavian, parsed_shavian, shavian_flags );

                    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "latin: %s, parsed latin: %s, latin flags:%04x"
                                                               , latin.c_str()
                                                               , parsed_latin.c_str()
                                                               , latin_flags );

                    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "shavian: %s, parsed shavian: %s, shavian flags:%04x"
                                                               , shavian.c_str()
                                                               , parsed_shavian.c_str()
                                                               , shavian_flags );

                    if ( latin_ok && shavian_ok )
                    {
                        STENO_ENTRY * steno_entry = new STENO_ENTRY();
                        
                        steno_entry->latin         = parsed_latin;
                        steno_entry->shavian       = shavian;
                        steno_entry->latin_flags   = latin_flags;
                        steno_entry->shavian_flags = shavian_flags;

                        //TEMP
                        //delay( 250 );

                        dictionary_->insert( std::make_pair( steno, * steno_entry ) );

                        entry_count++;
                    }
                    //TEMP
                    //if ( entry_count > 200 )
                    //{
                        //break;
                    //}
                }
                else
                {
                    non_data_count++;
                }
            }

            log_writeln_fmt( C_log::LL_VERBOSE_1, LOG_SOURCE, "%u entries loaded", entry_count );
            log_writeln_fmt( C_log::LL_VERBOSE_1, LOG_SOURCE, "%u non-data", non_data_count );

            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%u dictionary entries", dictionary_->size() );
        }

        return true;
    }
    catch ( std::exception & ex )
    {
        error_message_ = ex.what();
        
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Expection: %s", ex.what() );
    }
    catch( ... )
    {
    }

    return false;
}


// Output: text, shavian and flags are only set if the dictionary entry is found
bool
C_dictionary::lookup( const std::string & steno
                    , alphabet_type       alphabet
                    , std::string &       text
                    , uint16_t &          flags )
{
    auto result = dictionary_->find( steno );

    if ( result == dictionary_->end() )
    {
        return false;
    }
    else
    {
        std::string shavian = result->second.shavian;

        // If configured for Shavian, use the Shavian entry if it's not empty; otherwise use
        // the Latin entry.
        text  = ( alphabet == AT_SHAVIAN ) ? ( ( shavian.length() > 0 ) ? shavian : text) : text;
        flags = ( alphabet == AT_SHAVIAN ) ? result->second.shavian_flags : result->second.latin_flags;
    }

    return true;
}

bool
C_dictionary::get_first( STENO_ENTRY_2 & entry )
{
    it_ = dict_vector_->begin();

    if ( it_ != dict_vector_->end() )
    {
        entry = *it_;

        return true;
    }

    return true;
}

bool
C_dictionary::get_next( STENO_ENTRY_2 & entry )
{
    it_++;

    if ( it_ != dict_vector_->end() )
    {
        entry = *it_;

        return true;
    }

    return false;
}

bool
C_dictionary::parse_line( const std::string & line
                        , const char * regex
                        , std::string & field1
                        , std::string & field2
                        , std::string & field3 )
{
    std::regex regex_entry( regex );

    std::smatch matches;

    if ( std::regex_search( line, matches, regex_entry ) )
    {
        std::ssub_match match1 = matches[ 1 ];
        std::ssub_match match2 = matches[ 2 ];
        std::ssub_match match3 = matches[ 3 ];

        field1 = match1.str();
        field2 = match2.str();
        field3 = match3.str();

        return true;
    }

    return false;
}

/*
// Reading from an array of strings will cause escaped characters to appear as the character itself
// e.g. \\ becomes \
//      \"         "
// When writing these characters into new C string, they must be 're-escaped'
// i.e. \ becomes \\
//      " becomes \"
*/
void
C_dictionary::escape_characters( std::string & str )
{
    // NB: the "\\" entry must be first in the list
    //const char * esc_chars[] = { "\\", "\"", "\n", nullptr };
    const char * esc_chars[] = { "\n", nullptr };

    for ( uint32_t entry = 0; esc_chars[ entry ]; entry++ )
    {
        std::string from = esc_chars[ entry ];
        std::string to;
        
        if ( from == "\n" )
        {
            to = "\\n";
        }
        else
        {
            to = std::string( "\\" ) + esc_chars[ entry ];  
        }
 
        std::string::size_type pos = 0;

        while ( ( pos = str.find( from, pos ) ) != std::string::npos )
        {
            str.replace( pos, from.size(), to );
            pos += to.size();
        }
    }
}

}
