// miscellaneous.h
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

int
ctrl_to_text( const std::string & input, std::string & output );

void
delay( unsigned int milliseconds);

bool
file_exists( const std::string & path );

bool
directory_exists( const std::string & path );

bool
create_directory( const std::string & path );

}
