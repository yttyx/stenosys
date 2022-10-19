// buffer.h
// Implementation of circular buffer

#pragma once

#include <linux/types.h>

#include "mutex.h"
#include "thread.h"

//#define BUFFER_SIZE 256

namespace stenosys
{

template < class T, int L > 
class C_buffer
{

public:

    C_buffer()
    {
        put_index_ = 0;
        get_index_ = 0;
        count_     = 0;

        buffer_    = new T[ L ];
    }

    ~C_buffer()
    {
        delete [] buffer_;
        buffer_ = nullptr;
    }
    
    bool
    get( T & data )
    {
        mutex_.lock();

        if ( count_ > 0 )
        {
            data = buffer_[ get_index_ ];

            if ( ++get_index_ >= L )
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
    
    int 
    count()
    {
        int count = 0;

        mutex_.lock();
        count = count_;
        mutex_.unlock();

        return count;
    }

    bool
    put_block( const T * data, int length )
    {
        mutex_.lock();

        for ( int ii = 0; ii < length; ii++ )
        {
            if ( count_ < L )
            {
                buffer_[ put_index_ ] = data[ ii ];

                if ( ++put_index_ >= L )
                {
                    put_index_ = 0;
                }

                count_++;
            }
            else
            {
                mutex_.unlock();
                return false;
            }
        }

        mutex_.unlock();
        return true;
    }

    bool
    put( const T & data )
    {
        mutex_.lock();

        if ( count_ < L )
        {
            buffer_[ put_index_ ] = data;

            if ( ++put_index_ >= L )
            {
                put_index_ = 0;
            }

            count_++;

            mutex_.unlock();
            return true;    
        }

        mutex_.unlock();
        return false;
    }

private:
    
    T * buffer_;

    int  put_index_;
    int  get_index_;
    int  count_;

    C_mutex  mutex_;
};

}
