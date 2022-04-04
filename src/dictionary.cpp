#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <limits.h>

#include "cmdparser.h"
#include "dictionary.h"
#include "textfile.h"

namespace stenosys
{

const char * REGEX_DICTIONARY = "^.*\"(.*?)\": \"(.*?)\",$";  // JSON format line

C_dictionary::C_dictionary()
    : hashmap_( nullptr )
    , hash_capacity_( 0 )
    , hash_duplicate_count_( 0 )
    , hash_entry_count_( 0 )
    , hash_hit_capacity_count_( 0 )
    , hash_wrap_count_( 0 )
    , initialised_( false )
{
    // Analyse hash map collision distribution across 50 buckets
    hash_collision_distribution_ = std::make_unique< C_distribution >( "Collisions", 50, 1 );
}

C_dictionary::~C_dictionary()
{
    delete [] hashmap_;
}

bool
C_dictionary::build( const std::string & dictionary_path, const std::string & output_path )
{
    bool worked = true;

    worked = worked && read( dictionary_path );
    worked = worked && hash_map_build();
    worked = worked && hash_map_test();
    worked = worked && hash_map_report();
    worked = worked && write( output_path );
    
    return  worked;
}

// Read in JSON-format dictionary (Plover format) into an array of dictionary entries
bool
C_dictionary::read( const std::string & path )
{
    try
    {
        std::unique_ptr< C_text_file > text_file = std::make_unique< C_text_file >();

        if ( text_file->read( path ) )
        {
            std::string line;
            std::string steno;
            std::string text;

            std::cout << "Reading from " << get_filename( path ) << std::endl;

            uint32_t entry_count    = 0;
            uint32_t non_data_count = 0;
            
            std::stringstream text_stream_;

            while ( std::getline( text_stream_, line, '\n' ) )
            {
                if ( ( ++entry_count % 2000 ) == 0 )
                {
                    std::cout << "\r  Lines: " << entry_count;
                }

                // Check for valid JSON entry
                if ( text_file->parse_line( line, REGEX_DICTIONARY, steno, text ) )
                {
                    STENO_ENTRY steno_entry;

                    steno_entry.chord = steno;
                    steno_entry.text  = text;

                    dictionary_array_.push_back( steno_entry );
                }
                else
                {
                    non_data_count++;
                }
            }

            // Show final count
            // The non-data count should be 3:
            // - opening { of the JSON
            // - closing } of the JSON
            // - "WUZ/WUZ": "{}" dummy entry not having the trailing ','
            //   (required for JSON to load in Plover, I think)
            std::cout << "\r  Lines                : " << entry_count << std::endl;
            std::cout << "  Dictionary array size: " << dictionary_array_.size() << std::endl;
            std::cout << "  Non-data count       : " << non_data_count << " (should be 3)" << std::endl;

            return true;
        }
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
C_dictionary::hash_map_build()
{
    std::cout << "Building hash map" << std::endl;

    hash_map_initialise( dictionary_array_.size() );

    std::string key;
    std::string text;

    for ( uint32_t entry = 0; entry < dictionary_array_.size(); entry++ )
    {
        if ( get_dictionary_entry( entry, key, text ) )
        {
            uint32_t collisions = 0;
        
            if ( hash_insert( key, entry, collisions ) )
            {
                hash_collision_distribution_->add( collisions );
            }
            else
            {
                error_message_ = "Key: ";
                error_message_ += key;
                error_message_ += " insertion failure";

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
        // Multiply number of dictionary entrie by 1.75 to reduce the chance of
        // hash collisions. Trade-off is some wasted space vs reduced lookup times.
        hash_capacity_ = ( uint32_t ) ( (double ) dictionary_count * 1.75 );
        hashmap_       = new uint32_t[ hash_capacity_ ];
    
        // Initialise array
        for ( uint32_t ii = 0; ii < hash_capacity_; ii++ )
        {
            hashmap_[ ii ] = EMPTY;
        }

        initialised_ = true;
    }
}

// Add key/value pair
bool
C_dictionary::hash_insert( const std::string & key, uint32_t dictionary_entry, uint32_t & collisions )
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

        get_dictionary_entry( hashmap_[ hash_index ], map_key, value );
        
        if ( map_key != key )
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
    
    hashmap_[ hash_index ] = dictionary_entry;

    return true;
}

bool
C_dictionary::hash_map_test()
{
    std::cout << "Testing hash map" << std::endl;

    // Read sequentially through dictionary looking up each dictionary's entry using
    // the hash table: compare the sequential read against the hash read.
    uint32_t key_not_found            = 0;
    uint32_t value_mismatch           = 0;
    uint32_t dictionary_lookup_failed = 0;

    for ( uint32_t entry = 0; entry < dictionary_array_.size(); entry ++ )
    {
        std::string seq_key;
        std::string seq_value;

        if ( get_dictionary_entry( entry, seq_key, seq_value ) )
        {
            std::string lookup_value;

            if ( hash_find( seq_key, lookup_value ) )
            {
                if ( seq_value != lookup_value )
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

    std::cout << "  Keys not found            : " << key_not_found            << std::endl;
    std::cout << "  Dictionary lookup failures: " << dictionary_lookup_failed << std::endl;
    std::cout << "  Value mismatches          : " << value_mismatch           << std::endl;
    std::cout << "  Hash index test " << ( passed ? "passed" : "** failed" ) << std::endl;

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
        
        std::string map_key;
        
        get_dictionary_entry( hashmap_[ hash_index ], map_key, value );

        // If key found return its value
        if ( map_key == key )
        {
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
    std::cout << std::endl;
    std::cout << "Hash map report" << std::endl;
    std::cout << "---------------" << std::endl;
    std::cout << std::fixed << std::setprecision(6) << std::setfill(' ');
    std::cout << std::setw( 6 ) << "  hash_capacity_     : " << hash_capacity_ << std::endl;
    std::cout << std::setw( 6 ) << "  duplicate_count_   : " << hash_duplicate_count_ << std::endl;
    std::cout << std::setw( 6 ) << "  hash_entry_count_  : " << hash_entry_count_ << std::endl;
    std::cout << std::setw( 6 ) << "  hit_capacity_count_: " << hash_hit_capacity_count_ << std::endl;
    std::cout << std::setw( 6 ) << "  wrap_count_        : " << hash_wrap_count_ << std::endl;
    std::cout << std::endl;

    //std::string report = hash_collision_distribution_->report();
    //std::cout << report.c_str();

    return true;
}

bool
C_dictionary::get_dictionary_entry( uint32_t entry, std::string & steno, std::string & text )
{
    if ( entry < dictionary_array_.size() )
    {
        STENO_ENTRY steno_entry = dictionary_array_[ entry ];

        steno = steno_entry.chord;
        text  = steno_entry.text;

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
C_dictionary::write( const std::string & output_path )
{
    std::cout << "Writing out hashed dictionary to " << get_filename( output_path ) << std::endl;

    FILE * output_stream = nullptr;

    //TBW
    //fopen( &output_stream, output_path.c_str(), "w" );

    if ( output_stream != nullptr )
    {
        top( output_stream );
        write( output_stream );
        tail( output_stream );

        fclose ( output_stream );
    }
    else
    {
        std::cout << "Error accessing output file " << output_path.c_str() << std::endl;
        return false;
    }
    
    return true;
}

void
C_dictionary::write( FILE * output_stream )
{
    // Write the dictionary out in hash table order. This means that the dictionary is effectively
    // its own index and no separate hash table is required.

    fprintf( output_stream, "static uint32_t hash_table_length = %u;\n\n", hash_capacity_ );

    fprintf( output_stream, "struct dictionary_entry\n" );
    fprintf( output_stream, "{\n" );
    fprintf( output_stream, "    const char * const steno;\n" );
    fprintf( output_stream, "    const char * const text;\n" );
    fprintf( output_stream, "    const uint16_t     flags;\n" );
    fprintf( output_stream, "};\n\n" );
    
    fprintf( output_stream, "static const dictionary_entry steno_dictionary_hashed[] =\n" );
    fprintf( output_stream, "{\n" );

    for ( uint32_t entry = 0; entry < hash_capacity_; entry ++ )
    {
        if ( hashmap_[ entry ] != EMPTY )
        {
            std::string steno;
            std::string text;
           
            uint16_t flags = 0x0000;

            if ( get_dictionary_entry( hashmap_[ entry ], steno, text ) )
            {
                std::string text_in( text );
                std::string text_out;

                // Parse the dictionary text for Plover commands
                parser_.parse( text_in, text_out, flags );

                //TEMP
                if ( ( steno == "KW-GS" ) || ( steno == "R-R" ) || ( steno == "TPR-BGT" ) )
                {
                    int xxx = 666;
                    xxx++;
                }

                escape_characters( text_out );

                fprintf( output_stream, "    { \"%s\", \"%s\", 0x%04x },\n", steno.c_str(), text_out.c_str(), flags );
            }
        }
        else
        {
            fprintf( output_stream, "    { nullptr, nullptr, 0x0000 },\n" );
        }

        if ( ( entry % 400 ) == 0 )
        {
            std::cout << "\r" << entry << " entries written     ";
        }
    }

    std::cout << "\r" << hash_capacity_ << " entries written      \n";

    fprintf( output_stream, "};\n\n" );
}

void
C_dictionary::top( FILE * output_stream )
{
    fprintf( output_stream, "#include <cstdint>\n" );
    fprintf( output_stream, "#include <cstring>\n\n" );
}

const char * support_functions[] =
{
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
    "dictionary_lookup( const char * key, const char * & value, const uint16_t * & flags )",
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
    "            value = steno_dictionary_hashed[ hash_index ].text;",
    "            flags = &steno_dictionary_hashed[ hash_index ].flags;",
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
C_dictionary::tail( FILE * output_stream )
{
    for ( uint32_t ii = 0;  support_functions[ ii ]; ii++ ) 
    {
        fprintf( output_stream, "%s\n", support_functions[ ii ] );
    }
}

// Reading from an array of strings will cause escaped characters to appear as the character itself
// e.g. \\ becomes \
//      \"         "
// When writing these characters into new C string, they must be 're-escaped'
// i.e. \ becomes \\
//      " becomes \"
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

std::string
C_dictionary::get_filename( const std::string & path )
{
#if 0 //TBW
    char filename[ _MAX_FNAME ];
    char fileext[ _MAX_EXT ];

    _splitpath_s( path.c_str(), nullptr, 0, nullptr, 0, filename, sizeof( filename ), fileext, sizeof( fileext ) );

    std::string out = filename;
    out += fileext;

    return out;
#endif
}

}
