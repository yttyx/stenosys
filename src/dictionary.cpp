#include <cstdint>
#include <exception>
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
const char * OUTPUT_FILE_CPP  = "src/dictionary_i.cpp";
const char * OUTPUT_FILE_H    = "src/dictionary_i.h";


C_dictionary::C_dictionary()
    : initialised_( false )
    , hashmap_( nullptr )
    , hash_capacity_( 0 )
    , hash_entry_count_( 0 )
    , hash_wrap_count_( 0 )
    , hash_duplicate_count_( 0 )
    , hash_hit_capacity_count_( 0 )
{
    parser_     = std::make_unique< C_cmd_parser >();
    symbols_    = std::make_unique< C_symbols >();
    dictionary_ = std::make_unique< std::vector< STENO_ENTRY > >();

    // Analyse hash map collision distribution across 50 buckets
    distribution_ = std::make_unique< C_distribution >( "Collisions", 50, 1 );
}

C_dictionary::~C_dictionary()
{
    delete [] hashmap_;
}

bool
C_dictionary::build( const std::string & dictionary_path )
{
    bool worked = true;

    worked = worked && read( dictionary_path );
    worked = worked && hash_map_build();
    worked = worked && hash_map_test();
    worked = worked && hash_map_report();
    worked = worked && write_cpp();
    worked = worked && write_h();
    
    return  worked;
}

bool
C_dictionary::hash_map_build()
{
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "Building hash map" );
    
    hash_map_initialise( dictionary_->size() );

    for ( uint32_t index = 0; index < dictionary_->size(); index++ )
    {
        STENO_ENTRY entry;

        if ( get_dictionary_entry( index, entry ) )
        {
            uint32_t collisions = 0;
        
            if ( hash_insert( entry.steno, index, collisions ) )
            {
                distribution_->add( collisions );
            }
            else
            {
                log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Key '%s' insertion failure", entry.steno.c_str() );
                return false;
            }
        }
    }

    return true;
}

void
C_dictionary::hash_map_initialise( uint32_t dictionary_count )
{
    if ( ! initialised_ )
    {
        // Multiply number of dictionary entries by 1.75 to reduce the chance of
        // hash collisions. Trade-off is some wasted space vs reduced lookup times.
        hash_capacity_ = ( uint32_t ) ( (double ) dictionary_count * 1.75 );
        hashmap_       = new uint32_t[ hash_capacity_ ];
    
        // Initialise array
        for ( uint32_t index = 0; index < hash_capacity_; index++ )
        {
            hashmap_[ index ] = EMPTY;
        }

        initialised_ = true;
    }
}

// Add key/value pair
bool
C_dictionary::hash_insert( const std::string & key, uint32_t dictionary_index, uint32_t & collisions )
{
    if ( hash_entry_count_ >= hash_capacity_ )
    {
        hash_hit_capacity_count_++;
        return false;
    }

    collisions = 0;

    // Apply hash function to find index for the key
    uint32_t hash_index = generate_hash( key.c_str() );

    // Find next free space
    while ( hashmap_[ hash_index ] != EMPTY )   
    {
        std::string map_key;
        std::string value;

        STENO_ENTRY entry;

        get_dictionary_entry( hashmap_[ hash_index ], entry );
        
        if ( entry.steno != key )
        {
            collisions++;
            hash_index++;
            
            if ( hash_index >= hash_capacity_ )
            {
                hash_wrap_count_++;
            }

            // Wrap the index if required
            hash_index %= hash_capacity_;    
        }
        else
        {
            hash_duplicate_count_++;
            break;
        }
    }

    if ( hashmap_[ hash_index ] == EMPTY )
    {
        // A new entry is to be inserted, increment the entry count
        hash_entry_count_++;
    }
    
    hashmap_[ hash_index ] = dictionary_index;

    return true;
}

bool
C_dictionary::hash_map_test()
{
    std::cout << "Testing hash map" << std::endl;

    // Read sequentially through dictionary looking up each dictionary's entry using
    // the hash table; compare the sequential read against the hash read.
    uint32_t key_not_found            = 0;
    uint32_t value_mismatch           = 0;
    uint32_t dictionary_lookup_failed = 0;

    for ( uint32_t index = 0; index < dictionary_->size(); index++ )
    {
        std::string seq_key;
        std::string seq_value;

        STENO_ENTRY dict_entry;

        if ( get_dictionary_entry( index, dict_entry ) )
        {
            std::string hash_latin;

            if ( hash_find( dict_entry.steno, hash_latin ) )
            {
                if ( dict_entry.latin != hash_latin )
                {
                    value_mismatch++;
                }
            }
            else
            {
                key_not_found++;
            }
        }
        else
        {
            dictionary_lookup_failed++;
        }
    }

    bool passed = ( key_not_found == 0 ) && ( dictionary_lookup_failed == 0 )  && ( value_mismatch == 0 );

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  Keys not found            : %u", key_not_found );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  Dictionary lookup failures: %u", dictionary_lookup_failed );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  Value mismatches          : %u", value_mismatch );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  Hash index test           : %s ", ( passed ? "passed" : "FAILED" ) );

    return passed;
}

// Function to find the value for a given key
bool
C_dictionary::hash_find( const std::string & key, std::string & value )
{
    // Apply hash function to find the starting index for key
    uint32_t hash_index = generate_hash( key.c_str() );
    uint32_t counter    = 0;

    // From there, do a sequential search to find the key
    while ( hashmap_[ hash_index ] != EMPTY )
    {
        if ( counter++ > hash_capacity_ )
        {
            return false;
        }
        
        STENO_ENTRY dict_entry;

        get_dictionary_entry( hashmap_[ hash_index ], dict_entry );

        // If key found return its value
        if ( dict_entry.steno == key )
        {
            value = dict_entry.latin;
            return true;
        }

        hash_index++;
        hash_index %= hash_capacity_;
    }

    // Not found
    return false;
}

bool
C_dictionary::hash_map_report()
{
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "" );
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "Hash map report" );
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "---------------" );

    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  hash_capacity_     : %6u", hash_capacity_ );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  duplicate_count_   : %6u", hash_duplicate_count_ );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  hash_entry_count_  : %6u", hash_entry_count_ );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  hit_capacity_count_: %6u", hash_hit_capacity_count_ );
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  wrap_count_        : %6u", hash_wrap_count_ );
    
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "" );

    //std::string report = distribution_->report();
    //log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%s", report.c_str() );

    return true;
}

bool
C_dictionary::get_dictionary_entry( uint32_t      index
                                  , STENO_ENTRY & data )
{
    if ( index < dictionary_->size() )
    {
        data = dictionary_->at( index );

        return true;
    }

    return false;
}

// 'sdbm' hash
// http://www.cse.yorku.ca/~oz/hash.html#:~:text=If%20you%20just%20want%20to,K%26R%5B1%5D%2C%20etc.
uint32_t
C_dictionary::generate_hash( const char * key )
{
    uint32_t hash = 5381;
        
    int c;

    while ( ( c = *key++ ) != 0 )
    {
        hash = c + ( hash << 6 ) + ( hash << 16 ) - hash;
    }

    return hash % hash_capacity_;
}

bool
C_dictionary::write_cpp()
{
    std::cout << "Writing out hashed dictionary to " << OUTPUT_FILE_CPP << std::endl;

    FILE * output_stream = fopen( OUTPUT_FILE_CPP, "w" );

    if ( output_stream != nullptr )
    {

        write_cpp_top( output_stream );
        write_hash_table( output_stream );
        write_cpp_tail( output_stream );

        fclose( output_stream );
    }
    else
    {
        std::cout << "Error accessing output file " << OUTPUT_FILE_CPP << std::endl;
        return false;
    }
    
    return true;
}

bool
C_dictionary::write_h()
{
    std::cout << "Writing out hashed dictionary to " << OUTPUT_FILE_H << std::endl;

    FILE * output_stream = fopen( OUTPUT_FILE_H, "w" );

    if ( output_stream != nullptr )
    {
        header( output_stream );
    }
    else
    {
        std::cout << "Error accessing output file " << OUTPUT_FILE_H << std::endl;
        return false;
    }
    
    return true;
}



const char * cpp_top[] =
{
   "struct dictionary_entry",
   "{",
   "    const char *    const steno;" ,
   "    const char *    const latin;" ,
   "    const uint16_t  latin_flags;" ,
   "    const char *    const shavian;",
   "    const uint16_t  shavian_flags;",
   "};",
   "",
   "static const dictionary_entry steno_dictionary_hashed[] =",
   "{"
};

void
C_dictionary::write_cpp_top( FILE * output_stream )
{
    fprintf( output_stream, "static uint32_t hash_table_length = %u;\n\n", hash_capacity_ );
    
    for ( uint32_t ii = 0;  cpp_top[ ii ]; ii++ ) 
    {
        fprintf( output_stream, "%s\n", cpp_top[ ii ] );
    }
}

void
C_dictionary::write_hash_table( FILE * output_stream )
{
    // Write the dictionary out in hash table order. This means that the dictionary is
    // effectively its own index and no separate hash table is required.


    for ( uint32_t index = 0; index < hash_capacity_; index++ )
    {
        if ( hashmap_[ index ] != EMPTY )
        {
            STENO_ENTRY entry; 
            
            if ( get_dictionary_entry( hashmap_[ index ], entry ) )
            {
                std::string parsed_latin;
                std::string parsed_shavian;

                uint16_t latin_flags   = 0;
                uint16_t shavian_flags = 0;

                // Parse the dictionary text for Plover-style commands
                bool latin_ok   = parser_->parse( entry.latin,   parsed_latin,   latin_flags );
                bool shavian_ok = parser_->parse( entry.shavian, parsed_shavian, shavian_flags );

                if ( latin_ok && shavian_ok )
                {
                    escape_characters( parsed_latin );
                    escape_characters( parsed_shavian );

                    fprintf( output_stream, "    { \"%s\", u8\"%s\", 0x%04x, u8\"%s\", 0x%04x },\n"
                                          , entry.steno.c_str()
                                          , parsed_latin.c_str()
                                          , latin_flags
                                          , parsed_shavian.c_str()
                                          , shavian_flags );
                }
                else
                {            
                    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Invalid command in %s entry", entry.steno.c_str() );
                }
            }
        }
        else
        {
            fprintf( output_stream, "    { nullptr, nullptr, 0x0000, nullptr, 0x0000 },\n" );
        }
    }

    fprintf( output_stream, "};\n\n" );
    
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%u entries written",  hash_capacity_ );
}

const char * cpp_tail[] =
{
    "#include \"dictionary_i.h\"",
    "",
    "static uint32_t",
    "generate_hash( const char * key )",
    "{",
    "    uint32_t hash = 5381;",
     
    "    int c;",

    "    while ( ( c = *key++ ) != 0 )",
    "    {",
    "        hash = c + ( hash << 6 ) + ( hash << 16 ) - hash;",
    "    }",

    "    return hash % hash_table_length;",
    "}",
    "",
    "// Function to find the value for a given key",
    "bool",
    "dictionary_lookup( const char *       key",
    "                 , const char * &     latin",
    "                 , const uint16_t * & latin_flags",
    "                 , const char * &     shavian",
    "                 , const uint16_t * & shavian_flags )",
    "{",
    "    // Apply hash function to find index for given key",
    "    uint32_t hash_index = generate_hash( key );",
    "    uint32_t counter    = 0;",
    "",
    "    while ( steno_dictionary_hashed[ hash_index ].steno != nullptr )",
    "    {",
    "        if ( counter++ > hash_table_length )",
    "        {",
    "            return false;",
    "        }",
    "",
    "        const char * map_key = steno_dictionary_hashed[ hash_index ].steno;",

    "        if ( strcmp( map_key, key ) == 0 )",
    "        {",
    "            latin         = steno_dictionary_hashed[ hash_index ].latin;",
    "            latin_flags   = &steno_dictionary_hashed[ hash_index ].latin_flags;",
    "            shavian       = steno_dictionary_hashed[ hash_index ].shavian;",
    "            shavian_flags = &steno_dictionary_hashed[ hash_index ].shavian_flags;",
    "            return true;",
    "        }",

    "        hash_index++;",
    "", 
    "        // Wrap index if required",
    "        hash_index %= hash_table_length;",
    "    }",
    "",
    "    // Not found",
    "    return false;",
    "}",
    nullptr
};

void
C_dictionary::write_cpp_tail( FILE * output_stream )
{
    for ( uint32_t ii = 0;  cpp_tail[ ii ]; ii++ ) 
    {
        fprintf( output_stream, "%s\n", cpp_tail[ ii ] );
    }
}

const char * h[] =
{
    "#include <cstdint>",
    "#include <cstring>",
    "",
    "// Function to find the value for a given key",
    "bool",
    "dictionary_lookup( const char *       key",
    "                 , const char * &     latin",
    "                 , const uint16_t * & latin_flags",
    "                 , const char * &     shavian",
    "                 , const uint16_t * & shavian_flags )",
    "{",
    "};",
    nullptr
};

void
C_dictionary::write_h( FILE * output_stream )
{
    for ( uint32_t ii = 0;  h[ ii ]; ii++ ) 
    {
        fprintf( output_stream, "%s\n", h[ ii ] );
    }
}

// Read in tab-separated-value format dictionary (derived from Plover format) into an array of dictionary entries
bool
C_dictionary::read( const std::string & path )
{
    try
    {
        if ( C_text_file::read( path ) )
        {
            uint32_t entry_count    = 0;
            uint32_t bad_entry_count = 0;
            
            log_writeln( C_log::LL_INFO, LOG_SOURCE, "Reading dictionary" );
            
            std::string line;
    
            while ( get_line( line ) )
            {
                std::string steno;
                std::string latin;      // Latin alphabet
                std::string shavian;    // Shavian alphabet

                // Check for valid tab-separated-value entry
                if ( parse_line( line, REGEX_DICTIONARY, steno, latin, shavian ) )
                {
                    STENO_ENTRY * dict_entry = new STENO_ENTRY();
                        
                    dict_entry->steno   = steno;
                    dict_entry->latin   = latin;
                    dict_entry->shavian = shavian;

                    dictionary_->push_back( *dict_entry );

                    entry_count++;
                }
                else
                {
                    bad_entry_count++;
                }
            }

            log_writeln_fmt( C_log::LL_VERBOSE_1, LOG_SOURCE, "%u entries loaded", entry_count );
            log_writeln_fmt( C_log::LL_VERBOSE_1, LOG_SOURCE, "%u non-data", bad_entry_count );

            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%u dictionary entries", dictionary_->size() );
        }

        return true;
    }
    catch ( std::exception & ex )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Dictionary read exception: %s", ex.what() );
    }
    catch( ... )
    {
        log_writeln( C_log::LL_INFO, LOG_SOURCE, "Dictionary read exception" );
    }

    return false;
}


// TODO - superceded by hashad dictionary lookup
// Output: text, shavian and flags are only set if the dictionary entry is found
//bool
//C_dictionary::lookup( const std::string & steno
                    //, alphabet_type       alphabet
                    //, std::string &       text
                    //, uint16_t &          flags )
//{
    //auto result = dictionary_->find( steno );

    //if ( result == dictionary_->end() )
    //{
        ////log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Steno: %s not found", steno.c_str() );
        //return false;
    //}
    //else
    //{
        //std::string latin   = result->second.latin;
        //std::string shavian = result->second.shavian;

        ////log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "steno: %s found: shavian: %s, shavian flags: %04xh"
                                                     ////, steno.c_str()
                                                     ////, result->second.shavian.c_str()
                                                     ////, result->second.shavian_flags );

        //// If configured for Shavian, use the Shavian entry if it's not empty; otherwise use
        //// the Latin entry.
        //text  = ( alphabet == AT_SHAVIAN ) ? ( ( shavian.length() > 0 ) ? shavian : latin ) : latin;
        //flags = ( alphabet == AT_SHAVIAN ) ? result->second.shavian_flags : result->second.latin_flags;
    //}

    //return true;
//}

bool
C_dictionary::parse_line( const std::string & line
                        , const char * regex
                        , std::string & field1
                        , std::string & field2
                        , std::string & field3 )
{
    try
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
    }
    catch ( std::exception & ex )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Exception, dictionary line: %s: %s", line.c_str(), ex.what() );
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
    const char * esc_chars[] = { "\\", "\"", "\n", nullptr };
    //const char * esc_chars[] = { "\n", nullptr };

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
