/*
    Copyright (C) 2018  yttyx. This file is part of morsamdesa.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

// single.h
#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

namespace stenosys
{

template< class derived, class base >
class C_single
{
public:

    C_single();
    ~C_single();

    static std::shared_ptr< base > instance()    
    {
        if ( instance_.get() == nullptr )
        {
            instance_.reset( new derived() );
        }

        return instance_;
    }

protected:

    C_single( const C_single & ) {}

public:

    static std::shared_ptr< base > instance_;

};

template< class derived, class base >
std::shared_ptr< base > C_single< derived, base >::instance_ = nullptr;

}
