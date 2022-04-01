// geminipr.h
#pragma once

#include <string>
#include <stdint.h>

namespace stenosys
{

static const unsigned int BYTES_PER_STROKE = 6;

struct S_geminipr_packet
{
    uint8_t data[ BYTES_PER_STROKE ];

    void put( int index, uint8_t b );

    S_geminipr_packet &
    get();
};


class C_gemini_pr
{

private:

    C_gemini_pr() {}
    ~C_gemini_pr() {}

public:
    
    static std::string
    decode( S_geminipr_packet & packet );

    static const char steno_key_chart[];

};

}
