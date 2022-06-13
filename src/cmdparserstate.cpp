#include <cstring>
#include <stdio.h>

#include "cmdparser.h"
#include "log.h"
#include "state.h"
#include "cmdparserstate.h"
#include "utf8.h"

using namespace  stenosys;


namespace stenosys
{

// Initialise parser variables
STATE_DEFINITION( C_st_init, C_cmd_parser )
{
    fprintf( stdout, "C_st_init::handler() p: %p\n", p );
    fprintf( stdout, "  input_: %s\n", p->input_.c_str() );

    p->output_        = "";
    p->input_length_  = p->input_.length();
    p->bracket_count_ = 0;
    p->flags_         = 0;
    p->in_command_    = false;

    C_state::done_ = false;

    fprintf( stdout, "  input_.length(): %ld\n", p->input_.length() );
    
    set_state( p, C_st_find_command::s.instance(), "C_st_find_command" );
}

// Copy input text to the output until the start of a Plover command ("{") is found
STATE_DEFINITION( C_st_find_command, C_cmd_parser )
{
    std::string ch;

    // If there is a character, fetch it. It's a UTF-8 character, so it's returned as a string.
    if ( p->input_.peek_next( ch ) )
    {
        //fprintf( stdout, "ch: %s\n", ch.c_str() );

        if ( ch[ 0 ] == '{' )
        {
            // Found start of command
            p->input_.consume_next();
            set_state( p, C_st_got_command::s.instance(), "C_st_got_command" );
        }
        else
        {
            // Copy a character to the output and move on
            p->output_ += ch;
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
    //TEMP
    set_state( p, C_st_end::s.instance(), "C_st_end" );
}

STATE_DEFINITION( C_st_end, C_cmd_parser )
{
    C_state::done_ = true;
}

}
