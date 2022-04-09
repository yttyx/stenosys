#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

#include "cmdparser.h"
#include "dictionary.h"
#include "distribution.h"
#include "textfile.h"

namespace stenosys
{

#define EMPTY 0xffffffff

typedef struct {
    std::string chord;
    std::string text;
    uint16_t    flags;
} STENO_ENTRY;

class C_dictionary : C_text_file
{
public:

    C_dictionary();
    ~C_dictionary();

    bool
    read( const std::string & path );

private:


    bool
    lookup( uint32_t entry, std::string & steno, std::string & text, uint16_t flags );

    void
    escape_characters( std::string & str );

private:

    bool        initialised_;

    std::string error_message_;
    
    std::vector< STENO_ENTRY >          dictionary_array_;

    std::unique_ptr< C_command_parser > parser_;
};

}
