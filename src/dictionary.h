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
    std::string steno;
    std::string roman;
    std::string shavian;
} STENO_ENTRY;


class C_dictionary : C_text_file
{

public:

    C_dictionary();
    ~C_dictionary();

    bool
    build( const std::string & dictionary_path );   

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
    get_dictionary_entry( uint32_t      index
                        , STENO_ENTRY & data );

    uint32_t
    generate_hash( const char * key );

    bool
    write_cpp();
    
    bool
    write_h();
    
    void 
    header( FILE * output_stream );

    void
    top( FILE * output_stream );
    
    void
    write_cpp_top( FILE * output_stream );

    void
    write_hash_table( FILE * output_stream );

    void
    write_cpp_tail( FILE * output_stream );

    void
    write_header( FILE * output_stream );

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
    read( const std::string & path );

private:

    bool initialised_;

    std::unique_ptr< C_cmd_parser > parser_;
    std::unique_ptr< C_symbols >    symbols_;

    std::unique_ptr< std::vector< STENO_ENTRY > > dictionary_;
    std::unique_ptr< C_distribution >             distribution_;

    uint32_t * hashmap_;

    uint32_t hash_capacity_;
    uint32_t hash_entry_count_;

    uint32_t hash_wrap_count_;
    uint32_t hash_duplicate_count_;
    uint32_t hash_hit_capacity_count_;


    static const char * cpp_top[];
    static const char * cpp_tail[];
    static const char * hdr[];

    };

}
