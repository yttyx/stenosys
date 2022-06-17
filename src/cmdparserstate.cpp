#include <cstring>
#include <stdio.h>

#include "cmdparser.h"
#include "log.h"
#include "state.h"
#include "cmdparserstate.h"
#include "stenoflags.h"
#include "utf8.h"

using namespace  stenosys;


namespace stenosys
{

// Initialise parser variables
STATE_DEFINITION( C_st_init, C_cmd_parser )
{
    fprintf( stdout, "C_st_init::handler() p: %p\n", p );
    fprintf( stdout, "  input_: %s\n", p->input_.c_str() );

    p->output_         = "";
    p->input_length_   = p->input_.length();
    p->flags_          = 0;
    p->flags_internal_ = 0;
    p->in_command_     = false;
    p->got_text_       = false;

    C_state::done_ = false;

    fprintf( stdout, "  input_.length(): %ld\n", p->input_.length() );
    
    set_state( p, C_st_in_text::s.instance(), "C_st_in_text" );
}

// Copy input text to the output until the start of a Plover command ("{") is found
STATE_DEFINITION( C_st_in_text, C_cmd_parser )
{
    std::string ch;

    // If there is a character, fetch it. It's a UTF-8 character, so it's returned as a string.
    if ( p->input_.get_next( ch ) )
    {
        //fprintf( stdout, "ch: %s\n", ch.c_str() );

        if ( ch[ 0 ] == '{' )
        {
            // Found start of command
            set_state( p, C_st_got_command::s.instance(), "C_st_got_command" );
        }
        else
        {
            // Copy a character to the output and move on
            p->output_ += ch;
           
            // Encountering text cancels the following flags
            p->flags_ &= ( ~ ATTACH_TO_PREVIOUS );
            p->flags_ &= ( ~ ATTACH_TO_NEXT );
            p->flags_ &= ( ~ CAPITALISE_NEXT );
            p->flags_ &= ( ~ UPPERCASE_NEXT_WORD );
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
    // 1 char command e.g. ^ > & < 
    // {&} attach
    // {>} 
    // {.} output '.' <space> and uppercase next word
    // {,} output '.' <space> and uppercase next word
    // {?} output '?' <space> and uppercase next word
    // {!} output '!' <space> and uppercase next word
    //
    // 2 char command e.g. -|
    // no more chars
    // end of command '}'
    //
    std::string ch;

    if ( p->input_.get_next( ch ) )
    {
        switch ( ch[ 0 ] )
        {
            case '^':
                if (  ! p->got_text_ )
                {
                    p->flags_ |= ATTACH_TO_PREVIOUS;
                }

                p->flags_ |= ATTACH_TO_NEXT;
                
                set_state( p, C_st_get_command_end::s.instance(), "C_st_get_command_end" );
                break;

            case '.':
            case ',':
            case '?':
            case '!':
                p->output_ += ch;
                p->output_ += ' ';
                p->flags_  |= CAPITALISE_NEXT;
                break;

            case '<':
                p->flags_  |= UPPERCASE_NEXT_WORD;
                break;

            case '>':
                p->flags_  |= LOWERCASE_NEXT_WORD;
                break;

            case '-':
                // Two character command
                //set_state( p, C_st_got_command_2::s.instance(), "C_st_got_command_2" );

            case '}':
                // End of command
                set_state( p, C_st_in_text::s.instance(), "C_st_in_text" );
                break;
        
            default:
                //TODO Report command error
                set_state( p, C_st_end::s.instance(), "C_st_end" );
                break;
        }
    }
    else
    {
        set_state( p, C_st_end::s.instance(), "C_st_end" );
    }
}

// Search for valid command, whilst copying to the output any text which is not a command
STATE_DEFINITION( C_st_got_command_2, C_cmd_parser )
{
    // 2 character command e.g. -|
    // no more chars
    // end of command '}'
    //
    std::string ch;

    if ( p->input_.get_next( ch ) )
    {
        switch ( ch[ 0 ] )
        {
            case '|':
                p->flags_ |= CAPITALISE_NEXT;
                break;

            default:
                //TODO Report command error here
                set_state( p, C_st_end::s.instance(), "C_st_end" );
                break;
        }
    }
    else
    {
        //TODO Report command error here
        set_state( p, C_st_end::s.instance(), "C_st_end" );
    }
}

// Search for valid command, whilst copying to the output any text which is not a command
STATE_DEFINITION( C_st_get_command_end, C_cmd_parser )
{
    // We're now expected the end of the command
    std::string ch;

    if ( p->input_.get_next( ch ) )
    {
        if ( ch[ 0 ] == '}' )
        {
        }
        else
        {
            //TODO Report command error here
            set_state( p, C_st_end::s.instance(), "C_st_end" );
        }
    }
    else
    {
        set_state( p, C_st_end::s.instance(), "C_st_end" );
    }
}

STATE_DEFINITION( C_st_end, C_cmd_parser )
{
    C_state::done_ = true;
}

}
