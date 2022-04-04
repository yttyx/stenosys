#pragma once

#include <iostream>

namespace stenosys
{

class C_distribution
{

public:

    explicit
    C_distribution( const char * title, uint32_t max_bucket, uint64_t bucket_width );
  
    ~C_distribution();

    void
    add( uint64_t value );

    std::string
    report();

private:

    std::string
    format_percentage( double percentage );

public:

    uint32_t * buckets_;
    uint32_t   max_bucket_;
    uint32_t   max_used_bucket_;
    uint64_t   bucket_width_;

    std::string title_;
};

}