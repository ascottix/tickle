/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#ifndef EMU_STANDARD_MACHINE_H_
#define EMU_STANDARD_MACHINE_H_

#include <stdio.h>
#include <string.h>

#include "emu_driver.h"

class TStandardMachine : public TMachine
{
public:
    virtual ~TStandardMachine();

    virtual bool handleInputEvent( unsigned device, unsigned param, void * data = 0 );

protected:
    TStandardMachine();

    void createScreen( int width, int height, int colors );

    TBitmapIndexed * screen() {
        return screen_;
    }

    TPalette * palette() {
        return palette_;
    }

    TEventToPortHandler * eventHandler() {
        return &event_handler_;
    }

    TOptionToPortHandler * optionHandler() {
        return &option_handler_;
    }

    TJoystickToPortHandler * joystickHandler( int index ) {
        return joystick_handler_[index];
    }

    TResourceHandler * resourceHandler() {
        return &resource_handler_;
    }

    void setJoystickHandler( int index, TJoystickToPortHandler * jh ) {
        joystick_handler_[index] = jh;
    }

private:
    TBitmapIndexed *   screen_;
    TPalette *  palette_;    
    TEventToPortHandler     event_handler_;
    TOptionToPortHandler    option_handler_;
    TJoystickToPortHandler *joystick_handler_[4];
    TResourceHandler        resource_handler_;
};

#endif // EMU_STANDARD_MACHINE_H_
