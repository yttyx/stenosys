#pragma once

#include <memory>
#include <stdio.h>

#include "cmdparser.h"
#include "single.h"
#include "state.h"

//using namespace stenosys;

namespace stenosys
{

class C_cmd_parser;

class C_state_A : public C_state
{
public:

    static C_single< C_state_A, C_state > s;

protected:

    virtual void
    handler( C_cmd_parser * p ) override;

};

class C_state_B : public C_state
{
public:

    static C_single< C_state_B, C_state > s;

protected:

    virtual void
    handler( C_cmd_parser * p ) override;

};

class C_state_C : public C_state
{
public:

    static C_single< C_state_C, C_state > s;

protected:

    virtual void
    handler( C_cmd_parser * p ) override;

};

class C_state_D : public C_state
{
public:

    static C_single< C_state_D, C_state > s;

protected:

    virtual void
    handler( C_cmd_parser * p ) override;

};

}
