// config.h
#pragma once

#include <string>
#include <vector>

#include <libconfig.h++>

using namespace libconfig;

namespace stenosys
{

enum eOption
{
    OPT_NONE,
    OPT_DISPLAY_VERBOSITY,
    OPT_DISPLAY_DATETIME,
    OPT_FILE_STENO,
    OPT_FILE_DICT,
    OPT_DEVICE_RAW,
    OPT_DEVICE_STENO,
    OPT_DEVICE_OUTPUT
};

enum eFieldType
{
    FLD_NONE,
    FLD_STRING,
    FLD_INTEGER,
    FLD_BOOLEAN
};

struct S_option
{
    eOption       opt;
    eFieldType    field_type;
    int           field_offset;
    const char    *field_path;
    const char    *min;
    const char    *max;
};

struct S_config
{
    int  display_verbosity;
    bool display_datetime;
    
    std::string file_steno;
    std::string file_dict;

    std::string device_raw;
    std::string device_steno;
    std::string device_output;
};


class C_config
{
public:
    C_config();
    ~C_config();

    bool
    read( int argc, char *argv[] );

    const S_config &
    c() { return config_; }

protected:

    C_config( const C_config & ){}

    bool
    check_params( int argc, char *argv[] );

    void
    usage();

    bool
    read( std::string & filename );

    void
    display_settings();

    bool
    read_profile( const std::string & profile );
    
    void
    read_option_table( const Setting & parent, const S_option * options, void * record );

    bool
    setting_checks();

    bool
    setting_checks( const S_option * options, void * record );

    const char *
    to_string( bool setting );

    const char *
    to_string( eOption opt );

    std::string
    to_string( eOption opt1, eOption opt2 );

private:

    Config      cfg_;                   // libconfig instance
    S_config    config_;

    bool        got_config_file_;
    bool        display_config_;

    std::string config_file_;

};

}
