#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>

#include "cmdparserstate.h"
#include "state.h"

using namespace stenosys;

namespace stenosys
{

class C_cmd_parser
{
    friend class C_st_init;
    friend class C_st_find_command;
    friend class C_st_end;

public:

    C_cmd_parser();
    ~C_cmd_parser(){}

    void
    set_state( std::shared_ptr< C_state > state );

    void
    parse( const std::string & input, std::string & output, uint16_t & flags );

private:
    
    std::shared_ptr< C_state > state_;

    std::string input_;
    std::string output_;

    int pos_;
    int bracket_count_;
    int input_length_;

    bool        in_command_;
    uint16_t    flags_;
};

}
