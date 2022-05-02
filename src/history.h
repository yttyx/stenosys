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
    C_node * next;   // should this be C_node< T > *    ?
    C_node * prev;
    T *      o;
};

template < class T, int N > 
class C_history
{

public:

    C_history()
    {
        size_ = N;

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

        //clear();
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

        curr_->o.clear();
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
    
    uint16_t size_;

};

}