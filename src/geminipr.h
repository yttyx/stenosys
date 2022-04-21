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
         
    uint8_t & operator[]( std::size_t index );
    const uint8_t & operator[]( std::size_t index ) const;

//    void 
//    put( uint16_t index, uint8_t b );

    S_geminipr_packet *
    get();
};


class C_gemini_pr
{

public:
    
    static std::string
    decode( const S_geminipr_packet & packet );
    
    static S_geminipr_packet * 
    encode( const std::string & stroke );

    static const char steno_key_chart[];

private:

    C_gemini_pr() {}
    ~C_gemini_pr() {}

    static bool
    suppress_hyphen( const std::string & lhs, const std::string & rhs );
};

}
