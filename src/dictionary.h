#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

#include "dictionary.h"
#include "distribution.h"

namespace stenosys
{

#define EMPTY 0xffffffff

typedef struct {
    std::string chord;
    std::string text;
} STENO_ENTRY;

class C_dictionary
{
public:

    C_dictionary();
    ~C_dictionary();

    bool
    build( const std::string & dictionary_path,  const std::string & output_path );   

private:

    bool
    read( const std::string & path );

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
    serialise( FILE * output_stream );

    void
    tail( FILE * output_stream );

    void
    escape_characters( std::string & str );

    std::string
    get_filename( const std::string & path );

private:

    uint32_t * hashmap_;

    bool     initialised_;
    uint32_t hash_capacity_;
    uint32_t hash_duplicate_count_;
    uint32_t hash_entry_count_;
    uint32_t hash_hit_capacity_count_;
    uint32_t hash_wrap_count_;

    std::string error_message_;
    
    std::vector< STENO_ENTRY >        dictionary_array_;

    std::unique_ptr< C_distribution > hash_collision_distribution_;

    C_command_parser  parser_;
};

}
