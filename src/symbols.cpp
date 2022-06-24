#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdio.h>

#include "log.h"
#include "miscellaneous.h"
#include "stenoflags.h"
#include "symbols.h"
#include "utf8.h"

#define LOG_SOURCE "SYMBO"

// This class implements a subset of Emily's symbols
//  Ref: https://github.com/EPLHREU/emily-symbols
//       https://github.com/EPLHREU/emily-symbols/blob/main/emily-symbols-poster.pdf


using namespace  stenosys;

namespace stenosys
{

extern C_log log;

C_symbols::C_symbols()
{
    symbol_map_ = std::make_unique< std::unordered_map< std::string, std::string > >();
    
    symbol_map_->insert( std::make_pair( "FR",     "!¬↦¡"  ) );
    symbol_map_->insert( std::make_pair( "FP",     "\"“”„" ) );
    symbol_map_->insert( std::make_pair( "FRLG",   "#©®™"  ) );
    symbol_map_->insert( std::make_pair( "RPBL",   "$¥€£"  ) );
    symbol_map_->insert( std::make_pair( "FRPB",   "%‰‱φ"  ) );
    symbol_map_->insert( std::make_pair( "FBG",    "&∩∧∈"  ) );
    symbol_map_->insert( std::make_pair( "F",      "'‘’‚"  ) );
    symbol_map_->insert( std::make_pair( "FPL",    "([<{"  ) );
    symbol_map_->insert( std::make_pair( "RBG",    ")]>}"  ) );
    symbol_map_->insert( std::make_pair( "L",      "*∏§×"  ) );
    symbol_map_->insert( std::make_pair( "G",      "+∑¶±"  ) );
    symbol_map_->insert( std::make_pair( "B",      ",∪∨∉"  ) );
    symbol_map_->insert( std::make_pair( "PL",     "-−–—"  ) );
    symbol_map_->insert( std::make_pair( "R",      ".•·…"  ) );
    symbol_map_->insert( std::make_pair( "RP",     "/⇒⇔÷"  ) );
    symbol_map_->insert( std::make_pair( "LG",     ":∋∵∴"  ) );
    symbol_map_->insert( std::make_pair( "RB",     ";∀∃∄"  ) );
    symbol_map_->insert( std::make_pair( "PBLG",   "=≡≈≠"  ) );
    symbol_map_->insert( std::make_pair( "FPB",    "?¿∝‽"  ) );
    symbol_map_->insert( std::make_pair( "FRPBLG", "@⊕⊗∅"  ) );
    symbol_map_->insert( std::make_pair( "FB",     "\\Δ√∞" ) );
    symbol_map_->insert( std::make_pair( "RPG",    "^«»°"  ) );
    symbol_map_->insert( std::make_pair( "BG",     "_≤≥µ"  ) );
    symbol_map_->insert( std::make_pair( "P",      "`⊂⊃π"  ) );
    symbol_map_->insert( std::make_pair( "PB",     "|⊤⊥¦"  ) );
    symbol_map_->insert( std::make_pair( "FPBG",   "~⊆⊇˜"  ) );
}

bool
C_symbols::lookup( const std::string & steno, std::string & text, uint16_t & flags )
{
    // TODO
    // - Unique starter : SKWH
    // - Symbol variants: FRPBLG
    // - Attachment     : AO   
    // - Capitalisation : *
    // - Variant select : EU
    // - Repetition     : TS (out of scope for Stenosys)

    text  = "";
    flags = 0; 
   
    // Check for unique starter
    if ( steno.find( PUNCTUATION_STARTER ) == std::string::npos )
    {
        return false;
    }

    // Steno order: STKPWHRAO*EUFRPBLGTSDZ

    // Extract symbol variant from steno string
    size_t start = steno.find_first_of( PUNCTUATION_VARIANTS, STARTER_LEN );
    size_t end   = steno.find_last_of( PUNCTUATION_VARIANTS );
    
    //TEMP
    fprintf( stdout, "symbols start: %ld end: %ld\n", start, end );
    
    C_utf8 variants;

    if  ( ( start != std::string::npos ) && ( end != std::string::npos ) )
    {
        std::string variant_steno = steno.substr( start, end - start + 1 );
    
        //TEMP
        fprintf( stdout, "variant_steno: %s\n", variant_steno.c_str() );

        // Look up variant
        auto result = symbol_map_->find( variant_steno );

        if ( result == symbol_map_->end() )
        {
            //TEMP
            fprintf( stdout, "symbols return: 1\n" );
            return false;
        }

        variants = result->second;
    }
    else
    {
        //TEMP
        fprintf( stdout, "symbols return: 2\n" );
        return false;
    }

    // Find symbol variant
    bool got_e = ( steno.find_first_of( "E", STARTER_LEN ) != std::string::npos );
    bool got_u = ( steno.find_first_of( "U", STARTER_LEN ) != std::string::npos );

    int variant_index = 0; 

    if ( ( ! got_e ) && ( ! got_u ) )
    {
        variant_index = 0;
    }
    else if ( got_e && ( ! got_u ) )
    {
        variant_index = 1;
    }
    else if ( ( ! got_e ) && got_u )
    {
        variant_index = 2;
    }
    else
    {
        variant_index = 3;
    }

    std::string variant = variants.at( variant_index );

    //TEMP
    fprintf( stdout, "variant_index: %d  variant: %s\n", variant_index, variant.c_str() );
    
    // Set multiplier value
    int multiplier = 1;

    if ( steno.find( "T", STARTER_LEN) != std::string::npos )
    {
        multiplier = ( steno.find( "S", STARTER_LEN ) != std::string::npos ) ? 4 : 3;
    }
    else
    {
        multiplier = ( steno.find( "S", STARTER_LEN ) != std::string::npos ) ? 2 : 1;
    }
    
    //TEMP
    fprintf( stdout, "multiplier: %d\n", multiplier );

    while ( multiplier-- )
    {
        text += variant;
    }

    //TEMP
    fprintf( stdout, "text: %s\n", text.c_str() );
    
    //TEMP
    fprintf( stdout, "steno.substr( STARTER_LEN): %s\n", steno.substr( STARTER_LEN ).c_str() );
    
    // Set attachment and capitalisation flags as required
    if ( steno.find( "*", STARTER_LEN ) != std::string::npos )
    {
        flags |= CAPITALISE_NEXT;
    }

    flags |= ( ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT );

    if ( steno.find( "A", STARTER_LEN ) != std::string::npos )
    {
        flags &= ( ~ATTACH_TO_PREVIOUS );
    }

    if ( steno.find( "O", STARTER_LEN ) != std::string::npos )
    {
        flags &= ( ~ATTACH_TO_NEXT );
    }

    //TEMP
    fprintf( stdout, "flags: %04xh\n", flags );

    return true;
}

void
C_symbols::tests()
{
    log_writeln( C_log::LL_INFO, LOG_SOURCE, "Symbols tests" );

    for ( S_test_entry * entry = &test_entries[ 0 ]; strlen( entry->steno ) > 0; entry++ )
    {
        test( entry->steno, entry->expected_text, entry->expected_flags );
    }
}

void
C_symbols::test( const std::string & steno 
               , const std::string & expected_text
               , uint16_t            expected_flags )
{
    std::string text;
    
    uint16_t flags = 0;

    lookup( steno, text, flags );
   
    bool text_match  = ( expected_text == text );
    bool flags_match = ( expected_flags == flags );

    if ( ( ! text_match ) || ( ! flags_match ) )
    {
        log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "Steno: %s", steno.c_str() );
        
        if ( ! text_match )
        {
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  expected %s, got %s"
                                                         , ctrl_to_text( expected_text ).c_str()
                                                         , text.c_str() );
        }

        if ( ! flags_match )
        {
            log_writeln_fmt( C_log::LL_INFO, LOG_SOURCE, "  expected %04xh, got %04xh"
                                                         , flags
                                                         , expected_flags );
        }
    }
}

S_test_entry
C_symbols::test_entries[] = 
{   // steno        expected_text expected_flags
    { "SKHWFR",     "!",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWFRLG",   "#",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWRPBG",   "$",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWFRPB",   "%",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWFBG",    "&",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWF",      "'",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWFPL",    "(",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWRBB",    ")",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWL",      "*",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWG",      "+",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWB",      ",",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWPL",     "-",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWR",      ".",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWRP",     "/",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWLG",     ":",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWRB",     ";",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWPBLG",   "=",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWFPB",    "?",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWFRPBLG", "@",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWFB",     "\\",   ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWRPG",    "^",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWBG",     "_",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWP",      "`",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWPB",     "|",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "SKHWFPBG",   "~",    ATTACH_TO_PREVIOUS | ATTACH_TO_NEXT }
,   { "",           "",     0                                   }
};

}

