/*
    Copyright (C) 2018  yttyx. This file is part of morsamdesa.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

// timer.cpp

#include <assert.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "log.h"
#include "miscellaneous.h"
#include "timer.h"

#define LOG_SOURCE "TIMER"

using namespace stenosys;

namespace stenosys
{
extern C_log      log;

C_timer::C_timer()
{
    duration_ = 0;
    active_   = false;
    expired_  = false;
}

void
C_timer::start( long milliseconds )
{
    duration_ = milliseconds;
    active_   = true;

    clock_gettime( CLOCK_REALTIME, &start_time_ );
}

void
C_timer::stop()
{
    active_  = false;
    expired_ = false;
}

bool
C_timer::expired()
{
    struct timespec curr_time;
    struct timespec elapsed_time;

    clock_gettime( CLOCK_REALTIME, &curr_time );

    diff( start_time_, curr_time, elapsed_time );

    long elapsed_ms = ( elapsed_time.tv_sec * 1000 ) + ( elapsed_time.tv_nsec / 1000000 );

    expired_ = elapsed_ms >= duration_;

    if ( expired_ )
    {
        active_ = false;
    }

    return expired_;
}

// Returns elapsed time in mS
unsigned int
C_timer::elapsed_ms()
{
    if ( ! active_ )
    {
        return 0;
    }

    struct timespec curr_time;
    struct timespec elapsed_time;

    clock_gettime( CLOCK_REALTIME, &curr_time );

    diff( start_time_, curr_time, elapsed_time );

    return ( elapsed_time.tv_sec * 1000) + ( elapsed_time.tv_nsec / 1000000 );
}

void
C_timer::diff( const timespec & start, const timespec & end, timespec & elapsed )
{
    if ( ( end.tv_nsec - start.tv_nsec ) < 0 )
    {
        elapsed.tv_sec  = end.tv_sec-start.tv_sec - 1;
        elapsed.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    }
    else
    {
        elapsed.tv_sec  = end.tv_sec - start.tv_sec;
        elapsed.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
}

timespec
C_timer::current_time()
{
    struct timespec curr_time;

    clock_gettime( CLOCK_REALTIME, &curr_time );

    return curr_time;
}

// Returns elapsed time in seconds. Fractions of a second are ignored.
unsigned int
C_timer::elapsed_sec( const timespec & start_time )
{
    struct timespec curr_time;
    struct timespec elapsed_time;

    clock_gettime( CLOCK_REALTIME, &curr_time );

    diff( start_time, curr_time, elapsed_time );

    return ( elapsed_time.tv_sec );
}

string
C_timer::elapsed_str( const timespec & start_time )
{
    struct timespec curr_time;
    struct timespec elapsed_time;

    clock_gettime( CLOCK_REALTIME, &curr_time );

    diff( start_time, curr_time, elapsed_time );

    string age = format_string( "%2ldh %2ldm", elapsed_time.tv_sec / 3600, ( elapsed_time.tv_sec / 60 ) % 60 );

    return age;
}


}
