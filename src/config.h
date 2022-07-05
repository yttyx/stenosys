// config.h
#pragma once

#include <string>
#include <vector>

#include "textfile.h"

namespace stenosys
{

#define CONFIG_DIR            "~/.stenosys"
#define CONFIG_FILE           "config"
#define CONFIG_PATH           CONFIG_DIR "/" CONFIG_FILE

#define OPT_DISPLAY_VERBOSITY "verbosity"
#define OPT_DISPLAY_DATETIME  "datetime"
#define OPT_FILE_STENOFILE    "stenofile"
#define OPT_DICTIONARY        "dictionary"
#define OPT_RAW_DEVICE        "rawdevice"
#define OPT_STENO_DEVICE      "stenodevice"
#define OPT_SERIAL_OUTPUT     "serialoutput"

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

    bool
    check_params( int argc, char *argv[] );

private:

    S_config    config_;

    bool        got_config_file_;

};

}
