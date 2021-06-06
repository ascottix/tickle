/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#ifndef EMU_INPUT_H_
#define EMU_INPUT_H_

#include "emu_math.h"

enum TInputDeviceClass
{
    dcButton            = 0x00010000,
    dcTrigger           = 0x00020000,
    dcSwitch            = 0x00040000,
    dcJoystick          = 0x00080000,
    dcOption            = 0x00100000,
};

enum TInputDeviceType
{
    // Button/key
    dtKey               = 0x0000 | dcButton,
    dtKeyAction         = 0x0100 | dcButton,
    dtKeyStartPlayer    = 0x0200 | dcButton,
    dtKeyService        = 0x0300 | dcButton,
    // Trigger
    dtTrigger           = 0x0000 | dcTrigger,
    dtCoinSlot          = 0x0100 | dcTrigger,
    // Switch
    dtSwitch            = 0x0000 | dcSwitch,
    // Joystick
    dtJoystick          = 0x0000 | dcJoystick,
    dtTrackball         = 0x0100 | dcJoystick,
    dtMouse             = 0x0200 | dcJoystick,
    dtLightGun          = 0x0300 | dcJoystick,
    // Option
    dtDipSwitch         = 0x0000 | dcOption,
    dtDriverOption      = 0x0100 | dcOption,
    dtCheat             = 0x0200 | dcOption,
    dtBoardSwitch       = 0x0300 | dcOption,
};

enum TInputDevice
{
    idKeyStartPlayer1   = dtKeyStartPlayer | 0,
    idKeyStartPlayer2   = dtKeyStartPlayer | 1,

    idKeyP1Action1      = dtKeyAction | 0,
    idKeyP1Action2      = dtKeyAction | 1,
    idKeyP1Action3      = dtKeyAction | 2,
    idKeyP1Action4      = dtKeyAction | 3,

    idKeyP2Action1      = dtKeyAction | 4,
    idKeyP2Action2      = dtKeyAction | 5,
    idKeyP2Action3      = dtKeyAction | 6,
    idKeyP2Action4      = dtKeyAction | 7,

    idKeyService1       = dtKeyService | 0,
    idKeyService2       = dtKeyService | 1,
    idKeyService3       = dtKeyService | 2,
    idKeyService4       = dtKeyService | 3,

    idSwitchService1    = dtSwitch | 0,
    idSwitchService2    = dtSwitch | 1,
    idSwitchService3    = dtSwitch | 2,
    idSwitchService4    = dtSwitch | 3,

    idSwitchServiceTest = dtSwitch | 255,

    idCoinSlot1         = dtCoinSlot | 0,
    idCoinSlot2         = dtCoinSlot | 1,

    idJoyP1Joystick1    = dtJoystick | 0,
    idJoyP1Joystick2    = dtJoystick | 1,
    idJoyP2Joystick1    = dtJoystick | 2,
    idJoyP2Joystick2    = dtJoystick | 3,
};

enum TJoystickPosition 
{
    jpHalfRange = 2000,
    jpMaxPos    = +jpHalfRange,
    jpCenterPos = 0,
    jpMinPos    = -jpHalfRange,
    jpFullRange = 2*jpHalfRange + 1
};

class TInput
{
public:
    static unsigned getDeviceClass( unsigned device ) {
        return device & 0x7FFF0000;
    }

    static unsigned getDeviceType( unsigned device ) {
        return device & 0x7FFFFF00;
    }

    static unsigned getDeviceId( unsigned device ) {
        return device & 0x000000FF;
    }

    static int getXPosFromParam( unsigned param ) {
        return ((int)(param & 0xFFF)) - jpHalfRange;
    }

    static int getYPosFromParam( unsigned param ) {
        return ((int)((param >> 12) & 0xFFF)) - jpHalfRange;
    }

    static unsigned makeParamFromPos( int x, int y ) {
        x = TMath::clip( x, jpMinPos, jpMaxPos );
        y = TMath::clip( y, jpMinPos, jpMaxPos );

        return (unsigned)(x+jpHalfRange) + (((unsigned)(y+jpHalfRange)) << 12);
    }

private:
    TInput();
};

#endif // EMU_INPUT_H_
