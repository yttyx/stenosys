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
        size_ = N;
    }

    ~C_history(){}

    void
    add( T & obj )
    {
        deque_.push_front( obj );

        if ( deque_.size() > N )
        {
            deque_.pop_back();
        }
    }



private:
    

    std::deque< T > deque_;
    uint16_t        size_;

};

}
