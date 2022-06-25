#pragma once

#include "utf8.h"
#include <cstdint>
#include <string>
#include <stdio.h>

#include <memory>
#include <unordered_map>

//using namespace stenosys;

namespace stenosys
{

#define PUNCTUATION_STARTER  "SKWH"
#define PUNCTUATION_VARIANTS "FRPBLG"
#define STARTER_LEN          4

struct S_test_entry
{
    const char * steno;
    std::string expected_text;
    uint16_t expected_flags;
};


class C_symbols
{
public:

    C_symbols();
    
    virtual ~
    C_symbols() {}

    bool
    lookup( const std::string & steno, std::string & text, uint16_t & flags );


    void
    tests();

private:

    void
    test( const std::string & steno
        , const std::string & expected_text
        , uint16_t            expected_flags );

    bool
    get_symbols( const std::string & steno, C_utf8 & text );

    int
    get_symbol_variant( const std::string & steno );

    int
    get_multiplier( const std::string & steno );

    void
    set_flags( const std::string & steno, uint16_t & flags );

private:

    std::unique_ptr< std::unordered_map< std::string, std::string > > symbol_map_;

    static S_test_entry test_entries[];
};

}
