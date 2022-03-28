// misc.h
#pragma once

#include <string>

namespace stenosys
{

long
file_length( const std::string & filename );

void
current_time(  tm & tm_struct );

std::string
format_time_hh_mm( const tm & tm_struct );

std::string
format_datetime();

std::string
format_string( const char * format, ... );

void
find_and_replace( std::string & source, std::string const & find, std::string const & replace );

void
ltrim_string( std::string & s );

void
rtrim_string( std::string & s );

void
delay( unsigned int milliseconds);

}
