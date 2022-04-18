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
#include "textfile.h"

#define LOG_SOURCE "DICT "

namespace stenosys
{

extern C_log log;


const char * REGEX_DICTIONARY = "^.*\"(.*?)\": \"(.*?)\",$";  // JSON format line

C_dictionary::C_dictionary()
    : initialised_( false )
{
    parser_     = std::make_unique< C_command_parser >();
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
            
            std::string line;
    
            while ( get_line( line ) )
            {
                std::string steno;
                std::string text;

                uint16_t    flags = 0;

                // Check for valid JSON entry
                if ( parse_line( line, REGEX_DICTIONARY, steno, text ) )
                {
                    std::string parsed_text;

                    // Parse the dictionary text for Plover commands and set steno flags
                    parser_->parse( text, parsed_text, flags );

                    STENO_ENTRY * steno_entry = new STENO_ENTRY();
                    
                    steno_entry->text  = parsed_text;
                    steno_entry->flags = flags;
                    
                    dictionary_->insert( std::make_pair( steno, * steno_entry ) );
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
    }
    catch( ... )
    {
    }

    return false;
}


// Output: text and flags are only set if the dictionary entry is found
bool
C_dictionary::lookup( const std::string & steno, std::string & text, uint16_t & flags )
{
    auto result = dictionary_->find( steno );
    // std::unordered_map< std::string, STENO_ENTRY >::iterator it = dictionary_->find( steno );

    if ( result == dictionary_->end() )
    {
        return false;
    }
    else
    {
        text  = result->second.text;
        flags = result->second.flags;
    }

    return true;
}

bool
C_dictionary::parse_line( const std::string & line, const char * regex, std::string & param1, std::string & param2 )
{
    std::regex regex_entry( regex );

    std::smatch match;

    if ( std::regex_search( line, match, regex_entry ) )
    {
        std::ssub_match match1 = match[ 1 ];
        std::ssub_match match2 = match[ 2 ];

        param1 = match1.str();
        param2 = match2.str();

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
