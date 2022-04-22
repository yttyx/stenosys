
#include <algorithm>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "common.h"
#include "dictionary.h"
#include "log.h"
#include "stenoflags.h"
#include "stroke.h"

#define LOG_SOURCE "STRK "

using namespace stenosys;

namespace stenosys
{

extern C_log log;

void
C_stroke::set_next( C_stroke * stroke )
{
    next_ = stroke;
}

C_stroke *
C_stroke::get_next()
{
    return next_;
}

void
C_stroke::set_prev( C_stroke * stroke )
{
    prev_ = stroke;
}

C_stroke *
C_stroke::get_prev()
{
    return prev_;
}

void
C_stroke::set_steno( const std::string & steno )
{
    steno_ = steno;
}

const std::string &
C_stroke::get_steno()
{
    return steno_;
}

void
C_stroke::set_translation( const std::string & translation )
{
    translation_ = translation;
}

const std::string &
C_stroke::get_translation()
{
    return translation_;
}

uint16_t
C_stroke::get_flags()
{
    return flags_;
}

void
C_stroke::set_seqnum( uint16_t seqnum )
{
    seqnum_ = seqnum;
}

uint16_t
C_stroke::get_seqnum()
{
    return seqnum_;
}

bool
C_stroke::get_extends()
{
    return seqnum_ != 0;
}

bool
C_stroke::get_superceded()
{
    return superceded_;
}

void
C_stroke::clear()
{
    steno_         = "";
    found_         = false;
    best_match_    = false;
    translation_   = "";
    flags_         = 0;
    seqnum_        = 0;
    superceded_    = false;
}

}
