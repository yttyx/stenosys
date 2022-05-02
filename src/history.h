// history.h
// Implementation of circular buffer

#pragma once

#include <algorithm>
#include <linux/types.h>
#include <deque>
#include <memory>


namespace stenosys
{

template < class T, int N > 
class C_history
{

public:

    C_history()
    {
        size_  = N;
        index_ = 0;
    }

    ~C_history(){}

    void
    add( const T & obj )
    {
        deque_.push_front( obj );

        if ( deque_.size() > N )
        {
            deque_.pop_back();
        }
    }

    T *
    get_current()
    {
        index_ = 0;
        
        if ( deque_.size() > 0 )
        {
            return &deque_[ index_ ];
        }

        return nullptr;
    }

    T * 
    get_previous()
    {
        index_++;
        
        if ( deque_.size() > index_ )
        {
            return &deque_[ index_ ];
        }

        return nullptr;
    }

    void
    clear()
    {
        deque_.clear();
        index_ = 0;
    }

private:
    
    std::deque< T > deque_;
    uint16_t        size_;
    uint16_t        index_;

};

}
