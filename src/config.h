// config.h
#pragma once

#include <string>
#include <vector>

#include "textfile.h"

namespace stenosys
{

#define CONFIG_DIR            ".stenosys"
#define CONFIG_FILE           "config"

#define OPT_DISPLAY_VERBOSITY "verbosity"
#define OPT_DISPLAY_DATETIME  "datetime"
#define OPT_FILE_STENOFILE    "stenofile"
#define OPT_DICTIONARY        "dictionary"
#define OPT_RAW_DEVICE        "rawdevice"
#define OPT_STENO_DEVICE      "stenodevice"
#define OPT_SERIAL_OUTPUT     "serialoutput"

#define DEF_DISPLAY_VERBOSITY "3"
#define DEF_DISPLAY_DATETIME  "true"
#define DEF_FILE_STENOFILE    "TBD"
#define DEF_DICTIONARY        "./dictionary/yttyx-dict.tsv"
#define DEF_RAW_DEVICE        "/dev/input/event3"
#define DEF_STENO_DEVICE      "/dev/ttyACM0"
#define DEF_SERIAL_OUTPUT     "/dev/ttyAMA0"

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


class C_config : public C_text_file
{
public:
    C_config();
    ~C_config();

    bool
    read( int argc, char *argv[] );

    const S_config &
    c() { return config_; }

protected:

    void
    create_default_config( const std::string & config_path );
    
    bool
    read_config( const std::string & config_path );

    bool
    check_params( int argc, char *argv[], std::string & cfg_path );

private:

    S_config    config_;

    bool        got_config_file_;

};

}
