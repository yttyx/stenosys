#include <fcntl.h>
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

using namespace stenosys;

namespace stenosys
{

extern C_log log;

const char * REGEX_DICTIONARY = "^(.*?)\t(.*?)\t(.*?)$";

C_dictionary::C_dictionary()
    : initialised_( false )
{
    parser_     = std::make_unique< C_cmd_parser >();
    symbols_    = std::make_unique< C_symbols >();
    dictionary_ = std::make_unique< std::unordered_map< std::string, STENO_ENTRY > >();
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

                    bool latin_ok   = parser_->parse( latin, parsed_latin, latin_flags );
                    bool shavian_ok = parser_->parse( shavian, parsed_shavian, shavian_flags );
                    
                    //TEMP
                    //if ( steno == "TPH-D" )
                    //{
                        ////TEMP
                        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "shavian: %s, parsed shavian: %s, shavian flags:%04x"
                                                                   //, shavian.c_str()
                                                                   //, parsed_shavian.c_str()
                                                                   //, shavian_flags );

                        //std::string latin_formatted; 
                        //std::string parsed_latin_formatted; 

                        //ctrl_to_text( latin, latin_formatted );
                        //ctrl_to_text( parsed_latin, parsed_latin_formatted );

                        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "latin: %s, parsed latin: %s, latin flags:%04x"
                                                                   //, latin_formatted.c_str()
                                                                   //, parsed_latin_formatted.c_str()
                                                                   //, latin_flags );

                        //std::string shavian_formatted; 
                        //std::string parsed_shavian_formatted; 
                        
                        //ctrl_to_text( shavian, shavian_formatted );
                        //ctrl_to_text( parsed_shavian, parsed_shavian_formatted );
                        
                        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "shavian: %s, parsed shavian: %s, shavian flags:%04x"
                                                                   //, shavian_formatted.c_str()
                                                                   //, parsed_shavian_formatted.c_str()
                                                                   //, shavian_flags );
                    //}

                    if ( ( ! latin_ok ) || ( ! shavian_ok ) )
                    {
                        log_writeln_fmt( C_log::LL_VERBOSE_1, LOG_SOURCE, "Command error, dictionary entry %u", entry_count );
                    }


                    if ( latin_ok && shavian_ok )
                    {
                        STENO_ENTRY * steno_entry = new STENO_ENTRY();
                        
                        steno_entry->latin         = parsed_latin;
                        steno_entry->shavian       = parsed_shavian;
                        steno_entry->latin_flags   = latin_flags;
                        steno_entry->shavian_flags = shavian_flags;

                        dictionary_->insert( std::make_pair( steno, * steno_entry ) );

                        entry_count++;
                    }
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
        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Steno: %s not found", steno.c_str() );
        return false;
    }
    else
    {
        std::string latin   = result->second.latin;
        std::string shavian = result->second.shavian;

        //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "steno: %s found: shavian: %s, shavian flags: %04xh"
                                                     //, steno.c_str()
                                                     //, result->second.shavian.c_str()
                                                     //, result->second.shavian_flags );

        // If configured for Shavian, use the Shavian entry if it's not empty; otherwise use
        // the Latin entry.
        text  = ( alphabet == AT_SHAVIAN ) ? ( ( shavian.length() > 0 ) ? shavian : latin ) : latin;
        flags = ( alphabet == AT_SHAVIAN ) ? result->second.shavian_flags : result->second.latin_flags;
    }

    return true;
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

void
C_dictionary::tests()
{
    symbols_->tests();
}

}
