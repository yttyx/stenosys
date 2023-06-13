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

// timer.h

#ifndef timer_H
#define timer_H

#include "time.h"

using namespace std;

namespace stenosys
{

class C_timer
{
public:

    static const unsigned int DAY_SECS = 24 * 60 * 60;

    C_timer();
    ~C_timer() {}

    void
    stop();

    void
    start( long milliseconds );

    bool
    expired();

    unsigned int
    elapsed_ms();

    bool
    active() { return active_; }

    static timespec
    current_time();

    static unsigned int
    elapsed_sec( const timespec & start_time );

    static string
    elapsed_str( const timespec & start_time );


private:

    static void
    diff( const timespec & start, const timespec & end, timespec & elapsed );

private:

    long            duration_;
    struct timespec start_time_;
    bool            active_;
    bool            expired_;
};

}

#endif // timer_H
