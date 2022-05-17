#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "cmdparser.h"
#include "dictionary.h"
#include "textfile.h"

namespace stenosys
{

#define EMPTY 0xffffffff

typedef struct
{
    std::string text;
    uint16_t    flags;
} STENO_ENTRY;

typedef struct {
    std::string steno;
    std::string text;
} STENO_ENTRY_2;

class C_dictionary : C_text_file
{
public:

    C_dictionary();
    ~C_dictionary();

    bool
    read( const std::string & path );

    bool
    lookup( const std::string & steno, std::string & text, uint16_t & flags );

    bool
    get_first( STENO_ENTRY_2 & entry );

    bool
    get_next( STENO_ENTRY_2 & entry );

private:

    void
    escape_characters( std::string & str );

    bool
    parse_line( const std::string & line, const char * regex, std::string & param1, std::string & param2 );

    private:

    bool        initialised_;

    std::string error_message_;
    
    std::unique_ptr< std::unordered_map< std::string, STENO_ENTRY > >  dictionary_;
    std::unique_ptr< C_command_parser >                                parser_;

    std::unique_ptr< std::vector< STENO_ENTRY_2 > >  dict_vector_;
    std::vector< STENO_ENTRY_2 >::iterator           it_;
};

}
