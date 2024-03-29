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

extern C_log log;

// Initialise parser variables
STATE_DEFINITION( C_st_init, C_cmd_parser )
{
    p->output_         = "";
    p->input_length_   = p->input_.length();
    p->flags_          = 0;
    p->flags_internal_ = 0;
    p->parsed_ok_      = true;
    p->got_text_       = false;

    C_state::done_ = false;

    // Check if the string contains a command start character
    if ( p->input_.find( CMD_DELIMITER ) )
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

    // If there is a character, fetch it. It's a UTF-8 character so we handle it as a string.
    if ( p->input_.get_next( ch ) )
    {
        if ( ch[ 0 ] == CMD_DELIMITER )
        {
            // Found start of command
            set_state( p, C_st_got_command::s.instance(), "C_st_got_command" );
        }
        else if ( ch[ 0 ] == '\\' )
        {
            // Got escaped character
            set_state( p, C_st_escaped_char::s.instance(), "C_st_escaped_char" );
            
            // Encountering text cancels the following flags
            p->flags_ &= ( ~ ATTACH_TO_NEXT );
            p->flags_ &= ( ~ CAPITALISE_NEXT );
            p->flags_ &= ( ~ UPPERCASE_NEXT_WORD );

            p->got_text_ = true;
        }
        else
        {
            // Copy a character to the output and move on
            p->output_ += ch;
           
            // Encountering text cancels the following flags
            p->flags_ &= ( ~ ATTACH_TO_NEXT );
            p->flags_ &= ( ~ CAPITALISE_NEXT );
            p->flags_ &= ( ~ UPPERCASE_NEXT_WORD );
            
            p->got_text_ = true;
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
    std::string ch;

    bool two_char_cmd = false;

    if ( p->input_.get_next( ch ) )
    {
        switch ( ch[ 0 ] )
        {
            case CMD_DELIMITER:
                // End of command
                p->parsed_ok_ = false;
                
                log_writeln_fmt( C_log::LL_INFO, "Empty command: %s", p->input_.c_str() );
                break;
        
            case '^':
                if ( ! p->got_text_ )
                {
                    p->flags_ |= ATTACH_TO_PREVIOUS;
                }

                p->flags_ |= ATTACH_TO_NEXT;
                break;
            
            case '.':
            case '?':
            case '!':
                p->output_ += ch;

                if ( ! p->got_text_ )
                {
                    p->flags_ |= ATTACH_TO_PREVIOUS;
                }

                p->flags_  |= CAPITALISE_NEXT;
                break;

            case ',':
            case ';':
            case ':':
            case '[':
            case ']':
            case '{':
            case '}':
                p->output_ += ch;
                
                if ( ! p->got_text_ )
                {
                    p->flags_ |= ATTACH_TO_PREVIOUS;
                }
                break;

            case '<':
                p->flags_ |= UPPERCASE_NEXT_WORD;
                break;

            case '>':
                p->flags_ |= LOWERCASE_NEXT_WORD;
                break;

            case '&':
                if ( p->input_.get_next( ch ) )
                {
                    p->output_ += ch;
                    p->flags_  |= GLUE;
                }
                else
                {
                    p->parsed_ok_ = false;
                
                    log_writeln_fmt( C_log::LL_INFO, "Missing character from glue command: %s", p->input_.c_str() );
                }
                break;

            case '-':
                // Two character command
                two_char_cmd = true;
                break;

            default:
                p->parsed_ok_ = false;
                
                log_writeln_fmt( C_log::LL_INFO, "Invalid command: %s", p->input_.c_str() );
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
    std::string ch;

    if ( p->input_.get_next( ch ) )
    {
        switch ( ch[ 0 ] )
        {
            case '|':
                p->flags_ |= CAPITALISE_NEXT;
                set_state( p, C_st_get_command_end::s.instance(), "C_st_get_command_end" );
                break;
            
            case '!':
                p->flags_ |= NAMING_DOT;
                set_state( p, C_st_get_command_end::s.instance(), "C_st_get_command_end" );
                break;


            default:
                p->parsed_ok_ = false;
                
                log_writeln_fmt( C_log::LL_INFO, "Invalid command: %s", p->input_.c_str() );
                set_state( p, C_st_end::s.instance(), "C_st_end" );
                break;
        }
    }
    else
    {
        p->parsed_ok_ = false;

        log_writeln_fmt( C_log::LL_INFO, "Incomplete command: %s", p->input_.c_str() );
        set_state( p, C_st_end::s.instance(), "C_st_end" );
    }
}

STATE_DEFINITION( C_st_escaped_char, C_cmd_parser )
{
    // Handle escape sequences
    std::string ch;

    if ( p->input_.get_next( ch ) )
    {
        switch ( ch[ 0 ] )
        {
            case 'n':
                p->output_ += '\n';
                break;
            
            case 'r':
                p->output_ += '\r';
                break;
            
            case 't':
                p->output_ += '\t';
                break;

            case '\\':
                p->output_ += '\\';
                break;

            default:
                p->parsed_ok_ = false;
                break;
        } 

        if ( p->parsed_ok_ )
        {
            set_state( p, C_st_in_text::s.instance(), "C_st_in_text" );
        }
        else
        {
            log_writeln_fmt( C_log::LL_INFO, "Invalid escape sequence: %s", p->input_.c_str() );
            set_state( p, C_st_end::s.instance(), "C_st_end" );
        }
    }
    else
    {
        p->parsed_ok_ = false;
        log_writeln_fmt( C_log::LL_INFO, "Missing character in escape sequence: %s", p->input_.c_str() );
        set_state( p, C_st_end::s.instance(), "C_st_end" );
    }
}

//TODO
STATE_DEFINITION( C_st_raw_command, C_cmd_parser )
{
}

// Search for valid command, whilst copying to the output any text which is not a command
STATE_DEFINITION( C_st_get_command_end, C_cmd_parser )
{
    // We're now expected the end of the command
    std::string ch;

    if ( p->input_.get_next( ch ) )
    {
        if ( ch[ 0 ] == CMD_DELIMITER )
        {
            set_state( p, C_st_in_text::s.instance(), "C_st_in_text" );
        }
        else
        {
            p->parsed_ok_ = false;

            log_writeln_fmt( C_log::LL_INFO, "Bad command format: %s", p->input_.c_str() );
            set_state( p, C_st_end::s.instance(), "C_st_end" );
        }
    }
    else
    {
        log_writeln_fmt( C_log::LL_INFO, "Incomplete command: %s", p->input_.c_str() );
        set_state( p, C_st_end::s.instance(), "C_st_end" );
    }
}

STATE_DEFINITION( C_st_end, C_cmd_parser )
{
    C_state::done_ = true;
}

}
