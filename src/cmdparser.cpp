#include <cstring>
#include <iostream>
#include <string>
#include <regex>

#include "cmdparser.h"
#include "log.h"
#include "stenoflags.h"

#define LOG_SOURCE "PARSE"

using namespace std::regex_constants;

namespace stenosys
{

extern C_log      log;

const char * REGEX_COMMAND   = "\\{(.*?)\\}";

static bool trace = false;

typedef struct {
    const bool         regex;               // true: indicates entry is a regular expression; false: do string comparison
    const char * const command;
    uint16_t           flags;
} plover_command;

// NB: The order of the regex commands within the table is significant in some cases
const plover_command plover_commands[] =
{  
    { false, "{-|}",                    CAPITALISE_NEXT                                     }   // capitalise first letter of next word
,   { false,  "{~|}",                   CAPITALISE_NEXT                                     }
,   { false, "{*-|}",                   CAPITALISE_LAST                                     }   // capitalise last word
,   { false, "{<}",                     UPPERCASE_NEXT_WORD                                 }   // uppercase next word
,   { false, "{*<}",                    UPPERCASE_LAST_WORD                                 }   // uppercase previous word
,   { false, "{>}",                     LOWERCASE_NEXT_WORD                                 }   // lowercase next word
,   { false, "{*>}",                    LOWERCASE_LAST_WORD                                 }   // lowercase previous word
,   { false, "{^}",                     ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT                 }   // non-orthographic-aware attach


,   { true,  "\\{(;?)\\}",              ATTACH_TO_PREVIOUS | EMIT_SPACE                     }
,   { true,  "\\{(:?)\\}",              ATTACH_TO_PREVIOUS | EMIT_SPACE                     }

,   { true,  "\\{~\\|(.+?)\\^\\}",      ATTACH_TO_NEXT                                      }
,   { true,  "\\{\\^~\\|(.+?)\\}",      ATTACH_TO_PREVIOUS                                  }
,   { true,  "\\{\\^~\\|(.+?)\\^\\}",   ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT                 }
,   { true,  "\\{~\\|(.+?)\\}",         0                                                   }
    

,   { true,  "\\{\\^(.+?)\\^\\}",       ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT                 }
,   { true,  "\\{\\^(.+?)\\}",          ATTACH_TO_PREVIOUS                                  }   // orthographic-aware attach e.g. {^ing}"  (not supported) 
,   { true,  "\\{(.+?)\\^\\}",          ATTACH_TO_NEXT                                      }   // orthographic-aware attach e.g. {in^}"   (not supported) 
,   { true,  "\\{&(.+?)\\}",            GLUE                                                }   // glue operator e.g. {&x} or {&ab} 
,   { true,  "\\{(\\.)\\}",             ATTACH_TO_PREVIOUS | EMIT_SPACE | CAPITALISE_NEXT   }
,   { true,  "\\{(,)\\}",               ATTACH_TO_PREVIOUS | EMIT_SPACE                     }
,   { true, "\\{(!)\\}",                ATTACH_TO_PREVIOUS | EMIT_SPACE | CAPITALISE_NEXT   }
,   { true, "\\{(\\?)\\}",              ATTACH_TO_PREVIOUS | EMIT_SPACE | CAPITALISE_NEXT   }
,   { true,  "\\{#(.+?)\\}",            RAW                                                 }   // raw input e.g. {#Return}
,   { true,  "\\{(.+?)\\}",             EMIT_SPACE                                          }   // catch-all for {x} commands (table entry must come after the
 
,   { false, "",                    0                                                       }
};

C_command_parser::C_command_parser()
{
}

C_command_parser::~C_command_parser()
{
}

void
C_command_parser::parse( const std::string & text_in, std::string & text_out, uint16_t & flags )
{
    if ( contains( REGEX_COMMAND, text_in ) )
    {
        parse_command( text_in, text_out, flags );
    }
    else
    {
        text_out = text_in;
        flags    = 0x0000;
    }
}

// Parse a command. A steno dictionary entry might have one or more commands interspersed with text.
// Commands may have an effect on any text following the command.
void
C_command_parser::parse_command( const std::string & text_in, std::string & text_out, uint16_t & flags )
{
    try
    {
        std::regex  regex_cmd( REGEX_COMMAND );
        std::smatch match;

        //DEBUG
        //if ( text_in.find( "{^}-{^}" ) != std::string::npos )
        //{
            //trace = true;
        //}

        if ( trace )
        {
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "parse_command: text_in: %s", text_in.c_str() );
        }

        std::string::const_iterator search_start( text_in.cbegin() );
 
        text_out = "";

        if ( trace )
        {
            log_writeln( C_log::LL_INFO, LOG_SOURCE, "process_text: 1" );
        }
   
        std::string suffix;

        while ( regex_search( search_start, text_in.cend(), match, regex_cmd ) )
        {
            if ( trace )
            {
                log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "match.prefix: %s", match.prefix() );
                log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "match.suffix: %s", match.suffix() );
            }

            // Process text, either before the first command or between the current and previous commands
            process_text( match.prefix(), text_out, flags );
        
            if ( trace )
            {
                log_writeln( C_log::LL_INFO, LOG_SOURCE, "process_text: 2" );
            }
    
            process_command( match[ 0 ], text_out, flags );
            // Save match suffix in case there are no more commands to process
            // (the regex_search() call will clear match.suffix() in that case).
            suffix       = match.suffix();
            search_start = match.suffix().first;
        }
    
        if ( trace )
        {
            log_writeln( C_log::LL_INFO, LOG_SOURCE, "process_text: 3" );
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "match.suffix: %s", match.suffix() );
        }
    
        // Process any text following the final command
        process_text( suffix, text_out, flags );
    
        // Mask off internal formatting flags
        flags &= ( ~ INTERNAL_FLAGS_MASK );

        if ( trace )
        {
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "parse_command: text_out: %s", text_out.c_str() );
            trace = false;
        }
    }
    catch ( std::exception & ex )
    {
        std::string exc = ex.what();
        
        if ( trace )
        {
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "parse_command: exception: %s", exc.c_str() );
            trace = false;
        }
    }
    catch( ... )
    {
        if ( trace )
        {
            log_writeln( C_log::LL_INFO, LOG_SOURCE, "parse_command: exception (...)" );
            trace = false;
        }
    }
}

bool
C_command_parser::contains( const char * regex, const std::string & text )
{
    std::regex  search_regex( regex );
    std::smatch match;

    std::string::const_iterator search_start( text.cbegin() );
 
    return regex_search( search_start, text.cend(), match, search_regex );
}

void
C_command_parser::process_command( const std::string & command, std::string & text_out, uint16_t & flags )
{
    try
    {

        if ( trace )
        {
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "process_command: command: %s", command.c_str() );
        }



        for ( const plover_command * entry = plover_commands; entry->command[ 0 ] != '\0'; entry++ )
        {
            if ( entry->regex )
            {
                std::regex  regex_cmd( entry->command );
                std::smatch match;

                std::string::const_iterator search_start( command.cbegin() );

                if ( regex_search( search_start, command.cend(), match, regex_cmd ) )
                {
                    // For commands consisting of a sequence of commands interspersed with text
                    // flags are accumulated when a command is processed and flags may be 'eaten'
                    // and reset when text is processed.
                    flags |= entry->flags;

                    // Found the command - extract sub-match (if any) and add it to the text_out field
                    if ( match.size() == 2 )
                    {
                        if ( entry->flags & RAW )
                        {
                            lookup_raw( match[ 1 ], text_out, flags );
                        }
                        else
                        {
                            text_out += match[ 1 ];
                        }
                    }

                    if ( flags & GOT_TEXT )
                    {
                        flags &= ( ~ ATTACH_TO_PREVIOUS );
                    }

                    return;
                }
            }
            else
            {
                if ( strcmp( command.c_str(), entry->command ) == 0 )
                {
                    // Found it
                    flags |= entry->flags;

                    if ( flags & GOT_TEXT )
                    {
                        flags &= ( ~ ATTACH_TO_PREVIOUS );
                    }

                    return;
                }
            }
        }

        if ( trace )
        {
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "process_command: text_out: %s", text_out.c_str() );
        }

        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Command %s not supported", command.c_str() );
    }
    catch ( std::exception & ex )
    {
        std::string exc = ex.what();
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "parse_command: exception: %s", exc.c_str() );
    }
    catch( ... )
    {
    }
}

void
C_command_parser::process_text( std::string text_in, std::string & text_out, uint16_t & flags )
{
    if ( trace )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "process_text: text_in: %s", text_in.c_str() );
    }

    if ( text_in.length() > 0 )
    {
        // Check if text_in needs to be modified following the processing of a command. If so,
        // flags may need to be 'used up' and reset.
        if ( flags & EMIT_SPACE )
        {
            text_out += " ";
            flags &= ( ~ EMIT_SPACE );
        }

        // Remove any leading or trailing spaces
        text_in = std::regex_replace( text_in, std::regex( "^ +| +$|( ) +"), "$1" );


        if ( trace )
        {
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "process_text: text_in(2): %s", text_in.c_str() );
        }




        if ( flags & ATTACH_TO_NEXT )
        {
            flags &= ( ~ ATTACH_TO_NEXT );
        }
        if ( flags & CAPITALISE_NEXT )
        {
            change_case( CCT_UPPERCASE_LETTER, text_in );
            text_out += text_in;
            flags &= ( ~ CAPITALISE_NEXT );
        }
        if ( flags & LOWERCASE_NEXT )
        {
            change_case( CCT_LOWERCASE_LETTER, text_in );
            text_out += text_in;
            flags &= ( ~ LOWERCASE_NEXT );
        }
        if ( flags & UPPERCASE_NEXT_WORD )
        {
            change_case( CCT_UPPERCASE_WORD, text_in );
            text_out += text_in;
            flags &= ( ~ UPPERCASE_NEXT_WORD );
        }
        if ( flags & LOWERCASE_NEXT_WORD )
        {
            change_case( CCT_LOWERCASE_WORD, text_in );
            text_out += text_in;
            flags &= ( ~ LOWERCASE_NEXT_WORD );
        }

        // GOT_TEXT flag is checked by the command processing code
        // and used to clear the ATTACH_TO_PREVIOUS flag if a text
        // field is followed by a command.
        flags |= GOT_TEXT;

        text_out += text_in;
    }
}

void
C_command_parser::change_case( case_convert_type conversion_type, std::string & text_in )
{
    if ( text_in.length() > 0 )
    {
        switch ( conversion_type )
        {
            case CCT_UPPERCASE_LETTER:
                std::toupper( text_in[ 0 ] );
                break;
            case CCT_UPPERCASE_WORD:
                for ( size_t ii = 0; ( ii < text_in.length() ) && ( ! std::isspace( text_in[ ii ] ) ); ii++ )
                {
                    std::toupper( text_in[ ii ] );
                }
                break;
            case CCT_LOWERCASE_LETTER:
               std::tolower( text_in[ 0 ] );
               break;
            case CCT_LOWERCASE_WORD:
                for ( size_t ii = 0; ( ii < text_in.length() ) && ( ! std::isspace( text_in[ ii ] ) ); ii++ )
                {
                    std::tolower( text_in[ ii ] );
                }
                break;
        }
    }
}

void
C_command_parser::lookup_raw( const std::string & raw_command, std::string & text_out, uint16_t & flags )
{
    // TBW Add support for the raw definitions
    if ( raw_command == "Return" )
    {
        flags |= ATTACH_TO_PREVIOUS;
        text_out += "\n";
    }
}

}
