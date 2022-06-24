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

private:

    std::unique_ptr< std::unordered_map< std::string, std::string > > symbol_map_;

    static S_test_entry test_entries[];
};

}
