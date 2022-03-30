// gemini_pr.h
#pragma once

#include <poll.h>
#include <string>
#include <queue>

#include "mutex.h"
#include "thread.h"

namespace stenosys
{

class C_gemini_pr : public C_thread
{
    static const unsigned int BYTES_PER_STROKE = 6;

public:

    C_gemini_pr();
    ~C_gemini_pr();

    bool
    initialise( const std::string & device );
    
    bool
    start();

    void
    stop();

    bool
    read( std::string & str );

private:

    void
    thread_handler();

    bool
    get_byte( unsigned char & ch );
    
    std::string
    convert_stroke( const unsigned char packet[ BYTES_PER_STROKE ] );

    void
    add_if_unique( char key, std::string & stroke );

    bool
    suppress_hyphen( const std::string & lhs, const std::string & rhs );

    int
    set_interface_attribs( int fd, int speed );

private:

    int                 handle_;
    bool                abort_;

    std::queue< std::string > buffer_;
    C_mutex                   buffer_lock_;

    static const char   steno_key_chart[];
};

}

