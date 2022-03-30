// C_buffer: implemenation of circular buffer

#include <iostream>

#include <stdint.h>
#include <stdio.h>

#include "buffer.h"


using namespace stenosys;

namespace stenosys
{

C_buffer::C_buffer()
{
    handle_    = -1;
    abort_     = false;
    put_index_ = 0;
    get_index_ = 0;
    count_     = 0;
}

C_buffer::~C_buffer()
{
}


void
C_buffer::put()
{
    return thread_start();
}

bool
C_buffer::get( T & data )
{
    mutex_.lock();

    if ( count_ > 0 )
    {
        key_code = buffer_[ get_index_ ];

        if ( ++get_index_ >= BUFFER_SIZE )
        {
            get_index_ = 0;
        }

        count_--;
        
        mutex_.unlock();
        return true;
    }

    mutex_.unlock();
    return got_data;
}

bool
C_buffer::put( const T & data )
{
    mutex_.lock();

    if ( count_ < BUFFER_SIZE )
    {
        buffer_[ put_index_ ] = data;

        if ( ++put_index_ >= BUFFER_SIZE )
        {
            put_index_ = data;
        }

        count_++;

        mutex_.unlock();
        return false;    
    }

    mutex_.unlock();
    return true;
} 

}
