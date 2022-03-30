// C_buffer: implementation of circular buffer
#pragma once

#include <iostream>

#include <stdint.h>
#include <stdio.h>

#include "buffer.h"


using namespace stenosys;

namespace stenosys
{

template < class T >
C_buffer< T >::C_buffer()
{
    put_index_ = 0;
    get_index_ = 0;
    count_     = 0;
}

template < class T >
C_buffer< T >::~C_buffer()
{
}

template < class T >
bool
C_buffer< T >::get( T & data )
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

template < class T >
bool
C_buffer< T >::put( const T & data )
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
