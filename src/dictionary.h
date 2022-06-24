#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "cmdparser.h"
#include "dictionary.h"
#include "stenoflags.h"
#include "symbols.h"
#include "textfile.h"

using namespace stenosys;

namespace stenosys
{

#define EMPTY 0xffffffff

typedef struct
{
    std::string latin;
    uint16_t    latin_flags;
    std::string shavian;
    uint16_t    shavian_flags;
} STENO_ENTRY;

class C_dictionary : C_text_file
{

public:

    C_dictionary();
    ~C_dictionary();

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
    escape_characters( std::string & str );

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
    
    std::unique_ptr< std::unordered_map< std::string, STENO_ENTRY > >  dictionary_;
    std::unique_ptr< C_cmd_parser >                                    parser_;
    std::unique_ptr< C_symbols >                                       symbols_;
};

}
