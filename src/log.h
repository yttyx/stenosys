// log.h
#pragma once

#include "stdarg.h"
#include <string>

#include "mutex.h"

namespace stenosys
{

#define log_write( level, source, line )                log.write_line( level, __FILE__, __LINE__, source, false, line )
#define log_write_fmt( level, source, format, ...)      log.write_line( level, __FILE__, __LINE__, source, false, format, __VA_ARGS__ )
#define log_writeln( level, source, line )              log.write_line( level, __FILE__, __LINE__, source, true,  line )
#define log_writeln_fmt( level, source, format, ...)    log.write_line( level, __FILE__, __LINE__, source, true,  format, __VA_ARGS__ )

class C_log
{
public:

    enum eLogLevel
    {
        LL_NONE         = 0,
        LL_ERROR        = 1,
        LL_WARNING      = 2,
        LL_INFO         = 3,
        LL_VERBOSE_1    = 4,
        LL_VERBOSE_2    = 5,
        LL_VERBOSE_3    = 6
    };

    C_log();
    ~C_log();

    void
    initialise( int level, bool datetime );

    void
    write_line( eLogLevel level, const char * file, int line, const char * source, bool newline, const char * format, ... );

    eLogLevel
    log_level() { return level_; }

private:

    bool              datetime_;
    bool              fileline_;
    eLogLevel         level_;
                     
    C_mutex           log_lock_;

};

}