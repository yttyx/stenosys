// history.h
// Implementation of circular buffer

#pragma once

#include <linux/types.h>

#include <cstddef>
#include <cstdint>


namespace stenosys
{

template < class T > 
class C_node
{
public:
    C_node * next;
    C_node * prev;
    T *      o;
};

template < class T, int N > 
class C_history
{

public:

    C_history()
    {
        // Create a doubly-linked list of T objects
        C_node< T > * first = new_node();
        C_node< T > * prev = first;

        for ( std::size_t ii = 1; ii < N; ii++ )
        {
            C_node< T > * node = new_node();

            prev->next = node;
            node->prev = prev;
            
            prev = node;
        }
        
        // Complete the circular buffer
        prev->next = first;
        first->prev = prev;

        curr_ = first;
    }

    ~C_history()
    {
        for ( std::size_t ii = 0; ii < N; ii++ )
        {
            C_node< T > * node = curr_->next;

            delete curr_;
            curr_ = node;
        }
    }

    void
    add( const T & obj )
    {
        curr_ = curr_->next;

        bookmark_ = curr_;
        lookback_ = curr_;

        *curr_->o = obj;
    }

    T *
    curr()
    {
        return curr_->o;
    }

    T * 
    prev()
    {
        return curr_->prev->o;
    }

    T * 
    bookmark()
    {
        return bookmark_->o;
    }
    
    T * 
    bookmark_prev()
    {
        return bookmark_->prev->o;
    }

    T * 
    lookback()
    {
        return lookback_->o;
    }

    bool 
    go_back( T * & o )
    {
        // Back to but not including curr_
        if ( lookback_->prev != curr_ )
        {
            lookback_ = lookback_->prev;
            o = lookback_->o;
            return true;
        }

        return false;
    }
    
    bool 
    go_forward( T * & o )
    {
        if ( lookback_ != curr_ )
        {
            lookback_ = lookback_->next;
            o = lookback_->o;
            return true;
        }

        return false;
    }

    void
    reset_lookback()
    {
        lookback_ = curr_;
    }

    void
    set_bookmark()
    {
        bookmark_ = lookback_;
    }

    void goto_bookmark()
    {
        lookback_ = bookmark_;
    }

private:

    C_node< T > *
    new_node()
    {
        C_node< T > * node = new C_node< T >();

        node->o = new T();
        return node;
    }

public:


private:
    
    C_node< T > * curr_;
    C_node< T > * lookback_;
    C_node< T > * bookmark_;
};

}
