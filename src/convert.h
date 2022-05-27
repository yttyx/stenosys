// convert.h
#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include "dictionary.h"
#include "shaviandictionary.h"


using namespace stenosys;

namespace stenosys
{

class C_convert
{

public:

    C_convert(){}
    ~C_convert(){}

    bool
    convert( const std::string & steno_dict
           , const std::string & shavian_dict
           , const std::string & output_dict );

private:

private:

    std::unique_ptr< C_dictionary >         steno_dictionary_;
    std::unique_ptr< C_shavian_dictionary > shavian_dictionary_;

};

}
