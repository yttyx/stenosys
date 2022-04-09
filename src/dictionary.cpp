#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits.h>

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
    parser_ = std::make_unique< C_command_parser >();
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
        //temp
        log_writeln( C_log::LL_INFO, LOG_SOURCE, "C_dictionary::read()" );
    
        if ( C_text_file::read( path ) )
        {
            //temp
            log_writeln( C_log::LL_INFO, LOG_SOURCE, "C_dictionary::read(): 1" );

            uint32_t entry_count    = 0;
            uint32_t non_data_count = 0;
            
            std::string line;
    
            while ( get_line( line ) )
            {
                if ( ( ++entry_count % 2000 ) == 0 )
                {
                    std::cout << "\r  Lines: " << entry_count;
                }

                std::string steno;
                std::string text;

                uint16_t    flags = 0;

                // Check for valid JSON entry
                if ( parse_line( line, REGEX_DICTIONARY, steno, text ) )
                {
                    // Parse the dictionary text for Plover commands and set steno flags
                    parser_->parse( steno, text, flags );
                    
                    STENO_ENTRY steno_entry;
                    
                    steno_entry.chord = steno;
                    steno_entry.text  = text;
                    steno_entry.flags = flags;

                    dictionary_array_.push_back( steno_entry );
                }
                else
                {
                    non_data_count++;
                }
            }

            std::cout << "\n";
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
C_dictionary::lookup( uint32_t entry, std::string & steno, std::string & text, uint16_t flags )
{
    if ( entry < dictionary_array_.size() )
    {
        STENO_ENTRY steno_entry = dictionary_array_[ entry ];

        steno = steno_entry.chord;
        text  = steno_entry.text;
        flags = steno_entry.flags;

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
