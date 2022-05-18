
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

C_stroke::C_stroke()
{
    flags_       = 0;
    seqnum_      = 0;
}

C_stroke::C_stroke( const std::string & steno )
{
    steno_       = steno;
    translation_ = steno;

    flags_       = 0;
    seqnum_      = 0;
}

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
    translation_   = rhs.translation_;
    shavian_       = rhs.shavian_;
    flags_         = rhs.flags_;
    seqnum_        = rhs.seqnum_;

    return *this;
}

void
C_stroke::translation( const std::string & translation )
{
    translation_ = translation;
}

std::string &
C_stroke::translation()
{
    return translation_;
}

void
C_stroke::shavian( const std::string & shavian )
{
    shavian_ = shavian;
}

std::string &
C_stroke::shavian()
{
    return shavian_;
}

uint16_t
C_stroke::flags()
{
    return flags_;
}

void
C_stroke::flags( uint16_t flags )
{
    flags_ = flags;
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
    return seqnum_ > 1;
}

//bool
//C_stroke::superceded()
//{
    //return superceded_;
//}

void
C_stroke::clear()
{
    steno_         = "";
    translation_   = "";
    flags_         = 0;
    seqnum_        = 0;
}

}
