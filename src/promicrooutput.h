#pragma once

#include "keyevent.h"
#include "outputter.h"
#include "serial.h"

using namespace stenosys;

namespace stenosys
{

#define EV_KEY_UP                    0x00
#define EV_KEY_DOWN                  0x01
#define EV_KEY_AUTO                  0x02
#define EV_KEY_RELEASE_ALL           0x03   // Pseudo-command, used to ensure there are no 'stuck' keypresses
                                            // still issuing from the Pro Micro after stenosys is shut down
#define EV_KEY_NOOP                  0xff

// From the Arduino runtime source
#define ARDUINO_KEY_LEFT_CTRL       0x80
#define ARDUINO_KEY_LEFT_SHIFT      0x81
#define ARDUINO_KEY_LEFT_ALT        0x82
#define ARDUINO_KEY_LEFT_GUI        0x83
#define ARDUINO_KEY_RIGHT_CTRL      0x84
#define ARDUINO_KEY_RIGHT_SHIFT     0x85
#define ARDUINO_KEY_RIGHT_ALT       0x86
#define ARDUINO_KEY_UP_ARROW        0xDA
#define ARDUINO_KEY_DOWN_ARROW      0xD9
#define ARDUINO_KEY_LEFT_ARROW      0xD8
#define ARDUINO_KEY_RIGHT_ARROW     0xD7
#define ARDUINO_KEY_BACKSPACE       0xB2
#define ARDUINO_KEY_TAB             0xB3
#define ARDUINO_KEY_RETURN          0xB0
#define ARDUINO_KEY_ESC             0xB1
#define ARDUINO_KEY_INSERT          0xD1
#define ARDUINO_KEY_DELETE          0xD4
#define ARDUINO_KEY_PAGE_UP         0xD3
#define ARDUINO_KEY_PAGE_DOWN       0xD6
#define ARDUINO_KEY_HOME            0xD2
#define ARDUINO_KEY_END             0xD5
#define ARDUINO_KEY_CAPS_LOCK       0xC1
#define ARDUINO_KEY_F1              0xC2
#define ARDUINO_KEY_F2              0xC3
#define ARDUINO_KEY_F3              0xC4
#define ARDUINO_KEY_F4              0xC5
#define ARDUINO_KEY_F5              0xC6
#define ARDUINO_KEY_F6              0xC7
#define ARDUINO_KEY_F7              0xC8
#define ARDUINO_KEY_F8              0xC9
#define ARDUINO_KEY_F9              0xCA
#define ARDUINO_KEY_F10             0xCB
#define ARDUINO_KEY_F11             0xCC
#define ARDUINO_KEY_F12             0xCD

class C_pro_micro_output : public C_outputter
{
public:

    C_pro_micro_output();
    ~C_pro_micro_output();
    
    virtual bool    
    initialise();

    virtual void
    send( const std::string & str );
    
    virtual void
    send( uint16_t key_code );

    virtual void
    send( key_event_t key_event, uint8_t scancode );

    virtual void
    toggle_shavian();
    
    virtual void
    test();

    virtual void
    stop();

public:

    static unsigned char keytable[];

private:

    C_serial serial_;

};

}
