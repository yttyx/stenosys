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
                std::string text;      // Latin alphabet
                std::string shavian;    // Shavian

                uint16_t flags = 0;

                // Check for valid CSV entry
                if ( parse_line( line, REGEX_DICTIONARY, steno, text, shavian ) )
                {
                    C_utf8 parsed_text;

                    // Parse the dictionary text for Plover commands and set steno flags
                    parser_->parse( C_utf8( text ), parsed_text, flags );

                    STENO_ENTRY * steno_entry = new STENO_ENTRY();
                    
                    steno_entry->text    = parsed_text.str();
                    steno_entry->shavian = shavian;
                    steno_entry->flags   = flags;

                    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "text:%s, parsed text: %s, shavian:%s, flags:%u"
                                                               , text.c_str()
                                                               , parsed_text.c_str()
                                                               , shavian.c_str()
                                                               , flags );

                    //delay( 500 );

                    dictionary_->insert( std::make_pair( steno, * steno_entry ) );

                    //TEMP Add to dict_vector_ for later sequential access
                    //     (preserving the order of the entries in the file)
                    //STENO_ENTRY_2 * steno_entry_2 = new STENO_ENTRY_2();
                    
                    //steno_entry_2->steno = steno;
                    //steno_entry_2->text  = text;

                    //dict_vector_->push_back( *steno_entry_2 );
               
                    entry_count++;

                    if ( entry_count > 10 )
                    {
                        break;
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
                    , std::string &       text
                    , std::string &       shavian 
                    , uint16_t &          flags )
{
    auto result = dictionary_->find( steno );

    if ( result == dictionary_->end() )
    {
        return false;
    }
    else
    {
        text    = result->second.text;
        shavian = result->second.shavian;
        flags   = result->second.flags;
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
