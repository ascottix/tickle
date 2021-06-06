/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#include "emu_standard_machine.h"

TStandardMachine::TStandardMachine() 
{
    screen_ = 0;
    palette_ = 0;

    for( int i=0; i<4; i++ ) {
        joystick_handler_[i] = 0;
    }
}

TStandardMachine::~TStandardMachine() 
{
    delete screen_;
    delete palette_;

    for( int i=0; i<4; i++ ) {
        delete joystick_handler_[i];
    }
}

bool TStandardMachine::handleInputEvent( unsigned device, unsigned param, void * data ) 
{
    return 
        ((joystick_handler_[0] != 0) && (joystick_handler_[0]->handleInputEvent(device,param)) ) ||
        ((joystick_handler_[1] != 0) && (joystick_handler_[1]->handleInputEvent(device,param)) ) ||
        ((joystick_handler_[2] != 0) && (joystick_handler_[2]->handleInputEvent(device,param)) ) ||
        ((joystick_handler_[3] != 0) && (joystick_handler_[3]->handleInputEvent(device,param)) ) ||
        event_handler_.handleInputEvent(device,param) ||
        option_handler_.handleInputEvent(device,param);
}

void TStandardMachine::createScreen( int width, int height, int colors ) 
{
    palette_ = new TPalette( colors );
    screen_ = new TBitmapIndexed( width, height, palette_ );
}
