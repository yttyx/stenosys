// mutex.h

#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>

namespace stenosys
{

class C_mutex
{
public:

    C_mutex()
    {
        // Initialize to defaults
        pthread_mutex_init( &mutex_, nullptr);
    }

    virtual
    ~C_mutex()
    {
        pthread_mutex_destroy( &mutex_ );
    }

    int
    lock()
    {
        return pthread_mutex_lock( &mutex_ );
    }

    int
    try_lock()
    {
        return pthread_mutex_trylock( &mutex_ );
    }

    int
    unlock()
    {
        return pthread_mutex_unlock( &mutex_ );
    }

private:

    pthread_mutex_t     mutex_;

};

}

#endif  // MUTEX_H
