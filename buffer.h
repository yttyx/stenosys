// buffer.h
// Implementation of circular buffer

#pragma once

#include <linux/types.h>

#include "mutex.h"
#include "thread.h"

#define BUFFER_SIZE 256

namespace stenosys
{

template < class T > 
class C_buffer
{

public:

    C_buffer()
    {
        put_index_ = 0;
        get_index_ = 0;
        count_     = 0;
    }

    ~C_buffer(){}
    
    bool
    get( T & data )
    {
        mutex_.lock();

        if ( count_ > 0 )
        {
            data = buffer_[ get_index_ ];

            if ( ++get_index_ >= BUFFER_SIZE )
            {
                get_index_ = 0;
            }

            count_--;
            
            mutex_.unlock();
            return true;
        }

        mutex_.unlock();
        return false;
    }

    bool
    put( const T & data )
    {
        mutex_.lock();

        if ( count_ < BUFFER_SIZE )
        {
            buffer_[ put_index_ ] = data;

            if ( ++put_index_ >= BUFFER_SIZE )
            {
                put_index_ = 0;
            }

            count_++;

            mutex_.unlock();
            return false;    
        }

        mutex_.unlock();
        return true;
    }

private:
    
    T buffer_[ BUFFER_SIZE ];

    int  put_index_;
    int  get_index_;
    int  count_;

    C_mutex  mutex_;
};

}
