
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
C_stroke::steno( const std::string & steno )
{
    steno_ = steno;
}

const std::string &
C_stroke::steno()
{
    return steno_;
}

C_stroke &
C_stroke::operator=( const C_stroke & rhs )
{
    steno_         = rhs.steno_;
    found_         = rhs.found_;
    best_match_    = rhs.best_match_;
    translation_   = rhs.translation_;
    flags_         = rhs.flags_;
    seqnum_        = rhs.seqnum_;
    superceded_    = rhs.superceded_;

    return *this;
}

void
C_stroke::translation( const std::string & translation )
{
    translation_ = translation;
}

const std::string &
C_stroke::translation()
{
    return translation_;
}

uint16_t
C_stroke::flags()
{
    return flags_;
}

void
C_stroke::seqnum( uint16_t seqnum )
{
    seqnum_ = seqnum;
}

uint16_t
C_stroke::seqnum()
{
    return seqnum_;
}

bool
C_stroke::extends()
{
    return seqnum_ != 0;
}

bool
C_stroke::superceded()
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
