// thread.h
#pragma once

#include <pthread.h>

namespace stenosys
{

class C_thread
{
public:
    C_thread() {}
    virtual ~C_thread() {}

    // Returns true if the thread was successfully started, false if there was an error starting the thread
    bool thread_start()
    {
        return ( pthread_create( &thread_, nullptr, thread_entry_func, this ) == 0 );
    }

    // Will not return until the internal thread has exited
    void thread_await_exit()
    {
        (void) pthread_join( thread_, nullptr );
    }

protected:
    // Implement this method in your subclass with the code you want your thread to run
    virtual void thread_handler() = 0;

private:
    static void *thread_entry_func( void * me )
    {
        ( ( C_thread *) me )->thread_handler();
        return nullptr;
    }

private:
    pthread_t thread_;

};

}

