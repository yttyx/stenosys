#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>

#include "cmdparserstate.h"
#include "state.h"
#include "utf8.h"

using namespace stenosys;

namespace stenosys
{

class C_cmd_parser
{
    friend class C_st_init;
    friend class C_st_in_text;
    friend class C_st_got_command;
    friend class C_st_got_command_2;
    friend class C_st_get_command_end;
    friend class C_st_end;

public:

    C_cmd_parser();
    ~C_cmd_parser(){}

    void
    set_state( std::shared_ptr< C_state > state );

    bool
    parse( const std::string & input, std::string & output, uint16_t & flags );

private:
    
    std::shared_ptr< C_state > state_;

    C_utf8      input_;
    std::string output_;

    int         input_length_;

    bool        got_text_;
    bool        parsed_ok_;

    uint16_t    flags_;
    uint16_t    flags_internal_;
};

}
