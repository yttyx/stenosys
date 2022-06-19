#include <stdio.h>

#include "symbols.h"

// This class implements a subset of Emily's symbols
//  Ref: https://github.com/EPLHREU/emily-symbols
//       https://github.com/EPLHREU/emily-symbols/blob/main/emily-symbols-poster.pdf


using namespace  stenosys;

namespace stenosys
{

C_symbols::C_symbols()
{
    symbol_map_ = std::make_unique< std::unordered_map< std::string, std::string > >();
    
    symbol_map_->insert( std::make_pair( "FR",      "!¬↦¡"  ) );
    symbol_map_->insert( std::make_pair( "FP",      "\"“”„" ) );
    symbol_map_->insert( std::make_pair( "FRLG",    "#©®™"  ) );
    symbol_map_->insert( std::make_pair( "RPBL",    "$¥€£"  ) );
    symbol_map_->insert( std::make_pair( "FRPB",    "%‰‱φ"  ) );
    symbol_map_->insert( std::make_pair( "FBG",     "&∩∧∈"  ) );
    symbol_map_->insert( std::make_pair( "F",       "'‘’‚"  ) );
    symbol_map_->insert( std::make_pair( "FPL",     "([<{"  ) );
    symbol_map_->insert( std::make_pair( "RBG",     ")]>}"  ) );
    symbol_map_->insert( std::make_pair( "L",       "*∏§×"  ) );
    symbol_map_->insert( std::make_pair( "G",       "+∑¶±"  ) );
    symbol_map_->insert( std::make_pair( "B",       ",∪∨∉"  ) );
    symbol_map_->insert( std::make_pair( "PL",      "-−–—"  ) );
    symbol_map_->insert( std::make_pair( "R",       ".•·…"  ) );
    symbol_map_->insert( std::make_pair( "RP",      "/⇒⇔÷"  ) );
    symbol_map_->insert( std::make_pair( "LG",      ":∋∵∴"  ) );
    symbol_map_->insert( std::make_pair( "RB",      ";∀∃∄"  ) );
    symbol_map_->insert( std::make_pair( "PBLG",    "=≡≈≠"  ) );
    symbol_map_->insert( std::make_pair( "FPB",    "?¿∝‽"   ) );
    symbol_map_->insert( std::make_pair( "FRPBLG", "@⊕⊗∅"   ) );
    symbol_map_->insert( std::make_pair( "FB",     "\\Δ√∞"  ) );
    symbol_map_->insert( std::make_pair( "RPG",    "^«»°"   ) );
    symbol_map_->insert( std::make_pair( "BG",     "_≤≥µ"   ) );
    symbol_map_->insert( std::make_pair( "P",      "`⊂⊃π"   ) );
    symbol_map_->insert( std::make_pair( "PB",     "|⊤⊥¦"   ) );
    symbol_map_->insert( std::make_pair( "FPBG",   "~⊆⊇˜"   ) );
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
    // - Repitition     : TS (out of scope for Stenosys)
    return false;
}

}

