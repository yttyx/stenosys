#pragma once

#include <memory>
#include <stdio.h>

#include "single.h"

using namespace stenosys;

namespace stenosys
{

class C_cmd_parser;

class C_state
{
public:

    C_state();
    
    virtual ~
    C_state() {}

    virtual void
    handler( C_cmd_parser * p );

    bool
    done() { return done_; }

protected:

    void
    set_state( C_cmd_parser * test, std::shared_ptr< C_state > state, const char * description );

    static bool done_;

private:

};


#define STATE_DECLARATION( state_class, state_object )  \
    class state_class : public C_state                  \
    {                                                   \
    public:                                             \
                                                        \
        static C_single< state_class, C_state > s;      \
                                                        \
    protected:                                          \
                                                        \
        virtual void                                    \
        handler( state_object * p ) override;           \
                                                        \
    }

}


#define STATE_DEFINITION( state_class, state_object )   \
    void                                                \
    state_class::handler( state_object * p )            \

