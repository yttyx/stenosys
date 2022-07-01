#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "cmdparser.h"
#include "dictionary.h"
#include "distribution.h"
#include "stenoflags.h"
#include "symbols.h"
#include "textfile.h"

using namespace stenosys;

namespace stenosys
{

#define EMPTY 0xffffffff

// Used by direct lookup mechanism (which will be replaced)
typedef struct
{
    std::string latin;
    uint16_t    latin_flags;
    std::string shavian;
    uint16_t    shavian_flags;
} STENO_ENTRY_PREV;

typedef struct {
    std::string chord;
    std::string text;
} STENO_ENTRY;


class C_dictionary : C_text_file
{

public:

    C_dictionary();
    ~C_dictionary();

    bool
    build( const std::string & dictionary_path,  const std::string & output_path );   

    bool
    read( const std::string & path );

    bool
    lookup( const std::string & steno
          , alphabet_type       alphabet  
          , std::string &       text
          , uint16_t &          flags );
    void
    tests();

private:

    void
    hash_map_initialise( uint32_t dictionary_count );
    
    bool
    hash_map_build();


    bool
    hash_insert( const std::string & key, uint32_t dictionary_entry, uint32_t & collisions );

    bool
    hash_map_test();

    bool
    hash_map_report();

    bool
    hash_find( const std::string & key, std::string & value );  
    
    bool
    get_dictionary_entry( uint32_t entry, std::string & steno, std::string & text );

    uint32_t
    generate_hash( const char * key );

    bool
    write( const std::string & output_path );

    void
    top( FILE * output_stream );

    void
    write( FILE * output_stream );

    void
    tail( FILE * output_stream );

    void
    escape_characters( std::string & str );

    std::string
    get_filename( const std::string & path );

    bool
    parse_line( const std::string & line
              , const char * regex
              , std::string & field1
              , std::string & field2
              , std::string & field3 );

    bool
    parse_line( const std::string & line, const char * regex, std::string & param1, std::string & param2 );

private:

    bool        initialised_;

    std::string error_message_;
    
    std::unique_ptr< std::unordered_map< std::string, STENO_ENTRY_PREV > >  dictionary_;
    std::unique_ptr< C_cmd_parser >                                         parser_;
    std::unique_ptr< C_symbols >                                            symbols_;

    std::vector< STENO_ENTRY > dictionary_array_;

    uint32_t * hashmap_;

    uint32_t hash_capacity_;
    uint32_t hash_entry_count_;

    uint32_t hash_wrap_count_;
    uint32_t hash_duplicate_count_;
    uint32_t hash_hit_capacity_count_;

    std::unique_ptr< C_distribution > distribution_;

};

}
