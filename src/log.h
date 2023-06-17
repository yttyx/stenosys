// log.h
#pragma once

#include "stdarg.h"
#include <string>

#include "mutex.h"

namespace stenosys
{

#define log_write( level, line )                    log.write_line( level, true,  false, __FILE__, __LINE__, line )
#define log_write_fmt_raw( level, format, ... )     log.write_line( level, false, false, __FILE__, __LINE__, "", format, __VA_ARGS__ )
#define log_writeln_fmt_raw( level, format, ... )   log.write_line( level, false, true,  __FILE__, __LINE__, "", format, __VA_ARGS__ )
#define log_write_fmt( level, format, ...)          log.write_line( level, true,  false, __FILE__, __LINE__, format, __VA_ARGS__ )
#define log_writeln( level, line )                  log.write_line( level, true,  true,  __FILE__, __LINE__, line )
#define log_writeln_fmt( level, format, ...)        log.write_line( level, true,  true,  __FILE__, __LINE__, format, __VA_ARGS__ )

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
    initialise( eLogLevel level, bool datetime );

    void
    write_line( eLogLevel level, bool prefix_text, bool newline, const char * file, int line, const char * format, ... );

    eLogLevel
    log_level() { return level_; }

private:

    bool              datetime_;
    bool              fileline_;
    eLogLevel         level_;
                     
    C_mutex           log_lock_;

};

}
