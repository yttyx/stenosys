#pragma once

#include <memory>
#include <stdio.h>

#include "single.h"


namespace stenosys
{

class C_test;

class C_base_state
{
public:

    C_base_state();
    virtual ~C_base_state() {}

    virtual void
    handler( C_test * p );

    bool
    done() { fprintf( stdout, "C_base_state::done_: %d,  %p\n", done_, &C_base_state::done_ ); return done_; }

protected:

    void
    change_state_to( C_test * test, std::shared_ptr< C_base_state > state, const char * description );

    static bool done_;

private:

};

class C_state_A : public C_base_state
{
public:

    static C_single< C_state_A, C_base_state > s;

protected:

    virtual void
    handler( C_test * p ) override;

};

class C_state_B : public C_base_state
{
public:

    static C_single< C_state_B, C_base_state > s;

protected:

    virtual void
    handler( C_test * p ) override;

};

class C_state_C : public C_base_state
{
public:

    static C_single< C_state_C, C_base_state > s;

protected:

    virtual void
    handler( C_test * p ) override;

};

class C_state_D : public C_base_state
{
public:

    static C_single< C_state_D, C_base_state > s;

protected:

    virtual void
    handler( C_test * p ) override;

};

}
