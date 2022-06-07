#include <cstring>
#include <stdio.h>

#include "cmdparser.h"
#include "state.h"
#include "cmdparserstate.h"

using namespace  stenosys;


namespace stenosys
{

// Initialise parser variables
STATE_DEFINITION( C_st_init, C_cmd_parser )
{
    fprintf( stdout, "C_st_init::handler()\n" );

    p->input_length_  = p->input_.length();
    p->pos_           = 0;
    p->bracket_count_ = 0;
    p->flags_         = 0;
    p->in_command_    = false;

    set_state( p, C_st_find_command::s.instance(), "C_st_find_command" );
}

// Copy input text to the output until the start of a Plover command ("{") is found
// TODO Need to treat the dictionary text as UTF-8 strings when we come to parse
//      the Shavian dictionary entries (dual parsing the Latin and Shavian fields)
STATE_DEFINITION( C_st_find_command, C_cmd_parser )
{
    fprintf( stdout, "C_st_find_command::handler()\n" );

    // For now we handle the string as ASCII
    if ( p->pos_ < p->input_length_ )
    {
        if ( p->input_[ p->pos_ ] == '{' )
        {
            // Found start of command
        }
        else
        {
            // Copy a character to the output and move on
            p->output_ += p->input_[ p->pos_ ];
            p->pos_++;
        }
    }
    else
    {
        set_state( p, C_st_end::s.instance(), "C_st_end" );
    }
}

// Search for valid command, whilst copying to the output any text which is not a command
STATE_DEFINITION( C_st_got_command, C_cmd_parser )
{
    //TBW
    //TEMP goto end
    set_state( p, C_st_end::s.instance(), "C_st_end" );
}

STATE_DEFINITION( C_st_end, C_cmd_parser )
{
    fprintf( stdout, "C_st_end::handler()\n" );
   
    C_state::done_ = true;
}

}
