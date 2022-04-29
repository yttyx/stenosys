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

    bool
    get_current( T & obj )
    {
        index_ = 0;
        
        if ( deque_.size() > 0 )
        {
            obj = deque_[ index_ ];
            return true;
        }

        return false;
    }

    bool
    get_previous( T & obj )
    {
        index_++;
        
        if ( deque_.size() > index_ )
        {
            obj = deque_[ index_ ];
            return true;
        }

        return false;
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
