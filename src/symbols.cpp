#include <cstddef>
#include <stdio.h>

#include "stenoflags.h"
#include "symbols.h"
#include "utf8.h"

// This class implements a subset of Emily's symbols
//  Ref: https://github.com/EPLHREU/emily-symbols
//       https://github.com/EPLHREU/emily-symbols/blob/main/emily-symbols-poster.pdf


using namespace  stenosys;

namespace stenosys
{

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
    size_t start = steno.find_first_of( PUNCTUATION_VARIANTS );
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

    // The first character position after the punctuation starter
    int params_start = end + 1;

    // Find symbol variant
    bool got_e = ( steno.find_first_of( "E", params_start ) != std::string::npos );
    bool got_u = ( steno.find_first_of( "U", params_start ) != std::string::npos );

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

    if ( steno.find( "T", params_start) != std::string::npos )
    {
        multiplier = ( steno.find( "S", params_start ) != std::string::npos ) ? 4 : 3;
    }
    else
    {
        multiplier = ( steno.find( "S", params_start ) != std::string::npos ) ? 2 : 1;
    }
    
    //TEMP
    fprintf( stdout, "multiplier: %d\n", multiplier );

    while ( multiplier-- )
    {
        text += variant;
    }

    // Set attachment and capitalisation flags as required
    if ( steno.find( "*", params_start ) != std::string::npos )
    {
        flags |= CAPITALISE_NEXT;
    }

    if ( steno.find( "A", params_start ) != std::string::npos )
    {
        flags |= ATTACH_TO_PREVIOUS;
    }

    if ( steno.find( "O", params_start ) != std::string::npos )
    {
        flags |= ATTACH_TO_NEXT;
    }

    return true;
}

}

