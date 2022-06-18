#include <cstring>
#include <stdio.h>

#include "cmdparser.h"
#include "log.h"
#include "state.h"
#include "cmdparserstate.h"
#include "stenoflags.h"
#include "utf8.h"

#define LOG_SOURCE "CPARS"

using namespace  stenosys;

namespace stenosys
{

extern C_log log;

// Initialise parser variables
STATE_DEFINITION( C_st_init, C_cmd_parser )
{
    //fprintf( stdout, "C_st_init\n" );
    //fprintf( stdout, "  input_       : %s\n", p->input_.c_str() );
    //fprintf( stdout, "  input_length_: %ld\n", p->input_.length() );

    p->output_         = "";
    p->input_length_   = p->input_.length();
    p->flags_          = 0;
    p->flags_internal_ = 0;
    p->parsed_ok_      = true;
    p->got_text_       = false;

    C_state::done_ = false;

    // Check if the string contains a command start character
    if ( p->input_.find( "{" ) )
    {
        set_state( p, C_st_in_text::s.instance(), "C_st_in_text" );
    }
    else
    {
        // No command to parse
        p->output_ = p->input_.str();

        set_state( p, C_st_end::s.instance(), "C_st_end" );
    }
}

// Copy input text to the output until the start of a Plover command ("{") is found
STATE_DEFINITION( C_st_in_text, C_cmd_parser )
{
    std::string ch;

    // If there is a character, fetch it. It's a UTF-8 character so it's returned as a string.
    if ( p->get_next( ch ) )
    {
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

    bool two_char_cmd = false;

    if ( p->get_next( ch ) )
    {
        switch ( ch[ 0 ] )
        {
            case '^':
                if ( ! p->got_text_ )
                {
                    p->flags_ |= ATTACH_TO_PREVIOUS;
                }

                p->flags_ |= ATTACH_TO_NEXT;
                break;

            case ';':
            case ':':
            case '[':
            case ']':
                p->output_ += ch;
                p->output_ += ' ';
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
                p->flags_ |= UPPERCASE_NEXT_WORD;
                break;

            case '>':
                p->flags_ |= LOWERCASE_NEXT_WORD;
                break;

            case '&':
                if ( p->get_next( ch ) )
                {
                    p->output_ += ch;
                    p->flags_  |= GLUE;
                }
                else
                {
                    p->parsed_ok_ = false;
                
                    log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Missing character from glue command: %s", p->input_.c_str() );
                }
                break;

            case '-':
                // Two character command
                two_char_cmd = true;
                break;

            case '}':
                // End of command
                p->parsed_ok_ = false;
                
                log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Empty command: %s", p->input_.c_str() );
                break;
        
            default:
                p->parsed_ok_ = false;
                
                log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Invalid command: %s", p->input_.c_str() );
                break;
        }
        
        if ( two_char_cmd )
        {
            set_state( p, C_st_got_command_2::s.instance(), "C_st_got_command_2" );
        }
        else if ( p->parsed_ok_ )
        {
            set_state( p, C_st_get_command_end::s.instance(), "C_st_get_command_end" );
        }
        else
        {
            set_state( p, C_st_end::s.instance(), "C_st_end" );
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

    if ( p->get_next( ch ) )
    {
        switch ( ch[ 0 ] )
        {
            case '|':
                p->flags_ |= CAPITALISE_NEXT;
                set_state( p, C_st_get_command_end::s.instance(), "C_st_get_command_end" );
                break;

            default:
                p->parsed_ok_ = false;
                
                log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Invalid command: %s", p->input_.c_str() );
                set_state( p, C_st_end::s.instance(), "C_st_end" );
                break;
        }
    }
    else
    {
        p->parsed_ok_ = false;

        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Incomplete command: %s", p->input_.c_str() );
        set_state( p, C_st_end::s.instance(), "C_st_end" );
    }
}

// Search for valid command, whilst copying to the output any text which is not a command
STATE_DEFINITION( C_st_get_command_end, C_cmd_parser )
{
    // We're now expected the end of the command
    std::string ch;

    if ( p->get_next( ch ) )
    {
        if ( ch[ 0 ] == '}' )
        {
            set_state( p, C_st_in_text::s.instance(), "C_st_in_text" );
        }
        else
        {
            p->parsed_ok_ = false;

            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Bad command format: %s", p->input_.c_str() );
            set_state( p, C_st_end::s.instance(), "C_st_end" );
        }
    }
    else
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Incomplete command: %s", p->input_.c_str() );
        set_state( p, C_st_end::s.instance(), "C_st_end" );
    }
}

STATE_DEFINITION( C_st_end, C_cmd_parser )
{
    C_state::done_ = true;
}

}
