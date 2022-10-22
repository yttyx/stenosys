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
            std::string hash_roman;

            if ( hash_find( dict_entry.steno, hash_roman ) )
            {
                if ( dict_entry.roman != hash_roman )
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
            value = dict_entry.roman;
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
        write_header( output_stream );
        
        fclose( output_stream );
    }
    else
    {
        std::cout << "Error accessing output file " << OUTPUT_FILE_H << std::endl;
        return false;
    }
    
    return true;
}

void
C_dictionary::write_cpp_top( FILE * output_stream )
{
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "write_cpp_top()" );
    
    for ( uint32_t ii = 0;  cpp_top[ ii ]; ii++ ) 
    {
        fprintf( output_stream, "%s\n", cpp_top[ ii ] );
    }
    
    fprintf( output_stream, "static uint32_t hash_table_length = %u;\n\n", hash_capacity_ );
    fflush( output_stream );
}

void
C_dictionary::write_hash_table( FILE * output_stream )
{
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "write_hash_table()" );
    
    // Write the dictionary out in hash table order. This means that the dictionary is
    // effectively its own index and no separate hash table is required.

    fprintf( output_stream, "static const dictionary_entry steno_dictionary_hashed[] =\n" );
    fprintf( output_stream, "{\n" );

    for ( uint32_t index = 0; index < hash_capacity_; index++ )
    {
        if ( hashmap_[ index ] != EMPTY )
        {
            STENO_ENTRY entry; 
            
            if ( get_dictionary_entry( hashmap_[ index ], entry ) )
            {
                std::string parsed_roman;
                std::string parsed_shavian;

                uint16_t roman_flags   = 0;
                uint16_t shavian_flags = 0;

                // Parse the dictionary text for Plover-style commands
                bool roman_ok   = parser_->parse( entry.roman,   parsed_roman,   roman_flags );
                bool shavian_ok = parser_->parse( entry.shavian, parsed_shavian, shavian_flags );

                if ( roman_ok && shavian_ok )
                {
                    escape_characters( parsed_roman );
                    escape_characters( parsed_shavian );

                    fprintf( output_stream, "    { \"%s\", u8\"%s\", 0x%04x, u8\"%s\", 0x%04x },\n"
                                          , entry.steno.c_str()
                                          , parsed_roman.c_str()
                                          , roman_flags
                                          , parsed_shavian.c_str()
                                          , shavian_flags );
                }
                else
                {
                    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Invalid command in %s entry", entry.steno.c_str() );
                    //TEMP
                    break;
                }
            }
        }
        else
        {
            fprintf( output_stream, "    { nullptr, nullptr, 0x0000, nullptr, 0x0000 },\n" );
        }
    }

    fprintf( output_stream, "};\n\n" );
    fflush( output_stream );
    
    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "%u entries written",  hash_capacity_ );
}

void
C_dictionary::write_cpp_tail( FILE * output_stream )
{
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "write_cpp_tail()" );
    
    for ( uint32_t ii = 0;  cpp_tail[ ii ]; ii++ ) 
    {
        fprintf( output_stream, "%s\n", cpp_tail[ ii ] );
    }
    
    fflush( output_stream );
}

void
C_dictionary::write_header( FILE * output_stream )
{
    for ( uint32_t ii = 0;  hdr[ ii ]; ii++ ) 
    {
        fprintf( output_stream, "%s\n", hdr[ ii ] );
    }
    
    fflush( output_stream );
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
                std::string roman;      // roman alphabet
                std::string shavian;    // Shavian alphabet

                // Check for valid tab-separated-value entry
                if ( parse_line( line, REGEX_DICTIONARY, steno, roman, shavian ) )
                {
                    STENO_ENTRY * dict_entry = new STENO_ENTRY();
                        
                    dict_entry->steno   = steno;
                    dict_entry->roman   = roman;
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
// When writing these characters out into strings, they need to be 're-escaped'
// i.e. before    after
//      -----     ----
//        \        \\
//        "        \"
//      0x0a       \n
//      0x0d       \r
*/
void
C_dictionary::escape_characters( std::string & str )
{
    // NB: the "\\" entry must be first in the list
    const char * esc_chars[] = { "\\", "\"", "\r", "\n", nullptr };

    for ( uint32_t entry = 0; esc_chars[ entry ]; entry++ )
    {
        std::string from = esc_chars[ entry ];
        std::string to;
        
        if ( from == "\n" )
        {
            to = "\\n";
        }
        else if ( from == "\r" )
        {
            to = "\\r";
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

const char * C_dictionary::cpp_top[] =
{
    "#include <cstdint>",
    "#include <string>",
    "",
    "#include \"dictionary_i.h\"",
    "",
    "namespace stenosys",
    "{",
    "",
    "struct dictionary_entry",
    "{",
    "    const char *    const steno;" ,
    "    const char *    const roman;" ,
    "    const uint16_t  roman_flags;" ,
    "    const char *    const shavian;",
    "    const uint16_t  shavian_flags;",
    "};",
    "",
    nullptr
};

const char * C_dictionary::cpp_tail[] =
{
    "static uint32_t",
    "",
    "generate_hash( const char * key )",
    "{",
    "    uint32_t hash = 5381;",
    "",
    "    int c;",
    "",
    "    while ( ( c = *key++ ) != 0 )",
    "    {",
    "        hash = c + ( hash << 6 ) + ( hash << 16 ) - hash;",
    "    }",
    "",
    "    return hash % hash_table_length;",
    "}",
    "",
    "// Function to find the value for a given key",
    "bool",
    "dictionary_lookup( const char *       key",
    "                 , const char * &     roman",
    "                 , const uint16_t * & roman_flags",
    "                 , const char * &     shavian",
    "                 , const uint16_t * & shavian_flags )",
    "{",
    "    // Apply hash function to find index for given key",
    "    uint32_t hash_index = generate_hash( key );",
    "    uint32_t counter    = 0;",
    "",
    "    const dictionary_entry * base  = &steno_dictionary_hashed[ 0 ];",
    "    const dictionary_entry * entry = &steno_dictionary_hashed[ hash_index ];",
    "",
    "    while ( entry->steno != nullptr )",
    "    {",
    "        if ( ++counter >= ( hash_table_length - 1 ) )",
    "        {",
    "            return false;",
    "        }",
    "",
    "        if ( strcmp( entry->steno, key ) == 0 )",
    "        {",
    "            roman         = entry->roman;",
    "            roman_flags   = &entry->roman_flags;",
    "            shavian       = entry->shavian;",
    "            shavian_flags = &entry->shavian_flags;",
    "", 
    "            return true;",
    "        }",
    "",
    "        entry++;",
    "", 
    "        // Wrap if required",
    "        if ( ( ( uint32_t ) ( entry - base ) ) >= hash_table_length )",
    "        {",
    "            //TEMP",
    "            printf( \"** dictionary_lookup() wrap\\n\" );",
    "            entry = base;",
    "        }",
    "    }",
    "",
    "    // Not found",
    "    return false;",
    "}",
    "",
    "void",
    "word_lookup( const std::string & word, unsigned int max_words, std::list< std::string > & results )",
    "{",
    "",
    "    bool match_prefix = false;",
    "    bool match_suffix = false;",
    "",
    "    std::string search_word = word;",
    "",
    "    if ( ( word.front() == '*' ) && ( word.length() > 1 ) )",
    "    {",
    "        search_word  = word.substr( 1 );",
    "        match_suffix = true;",
    "    }",
    "    else if ( ( word.back() == '*' ) && ( word.length() > 1 ) )",
    "    {",
    "        search_word  = word.substr( 0, word.length() - 1 );",
    "        match_prefix = true;",
    "    }",
    "",
    "    fprintf( stdout, \"Find: %s\\n\", search_word.c_str() );",
    "    fprintf( stdout, \"  match_prefix = %s\\n\", match_prefix ? \"true\" : \"false\" );",
    "    fprintf( stdout, \"  match_suffix   = %s\\n\", match_suffix   ? \"true\" : \"false\" );",
    "",
    "    const dictionary_entry * entry = &steno_dictionary_hashed[ 0 ];",
    "",
    "    results.clear();",
    "",
    "    unsigned int word_count = 0;",
    "",
    "    // Brute force word lookup",
    "    for ( size_t index = 0; index < hash_table_length; index++, entry++ )",
    "    {",
    "        if ( ( entry->steno != nullptr ) && ( entry->roman != nullptr ) )",
    "        {",
    "            std::string roman = entry->roman;",
    "",
    "            bool found = false;",
    "",
    "            if ( match_prefix )",
    "            {",
    "                found = ( roman.find( search_word ) == 0 );",
    "            }",
    "            else if ( match_suffix )",
    "            {",
    "                std::string::size_type suffix_pos = roman.rfind( search_word );",
    "",            
    "                if ( suffix_pos != std::string::npos )",
    "                {",
    "                    found = ( suffix_pos == ( roman.length() - search_word.length() ) );",
    "                }",
    "            }",
    "            else",
    "            {",
    "                found = ( search_word == roman );",
    "            }",
    "",
    "            if ( found )",
    "            {",
    "                results.push_back( roman + std::string( \" \" ) + std::string( entry->steno ) );",
    "",
    "                if ( ++word_count >= max_words )",
    "                {",
    "                    break;",
    "                }",
    "            }",
    "        }",
    "    }",
    "",
    "}",
    "",
    "}  // namespace stenosys",
    nullptr
};

const char * C_dictionary::hdr[] =
{
    "#include <cstdint>",
    "#include <cstring>",
    "#include <list>",
    "",
    "namespace stenosys",
    "{",
    "",
    "// Function to find the value for a given key",
    "bool",
    "dictionary_lookup( const char *       key",
    "                 , const char * &     roman",
    "                 , const uint16_t * & roman_flags",
    "                 , const char * &     shavian",
    "                 , const uint16_t * & shavian_flags );",

    "void",
    "word_lookup( const std::string & word, unsigned int max_words, std::list< std::string > & results );",
    "",
    "}",
    nullptr
};

}
