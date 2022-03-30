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
class C_buffer : public C_thread
{

public:

    C_buffer();
    ~C_buffer();
    
    bool
    get( T & data );

    bool 
    put( const T & data );

private:
    
    T buffer_[ BUFFER_SIZE ];

    int  put_index_;
    int  get_index_;
    int  count_;

    C_mutex  mutex_;
};

}

#include "buffer.cpp"
