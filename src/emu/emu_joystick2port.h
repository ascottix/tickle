/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#ifndef EMU_JOYSTICK2PORT_H_
#define EMU_JOYSTICK2PORT_H_

#include "emu_event2port.h"
#include "emu_input.h"
#include "emu_math.h"

enum TJoystickPort {
    jpLeft      = 0,
    jpRight     = 1,
    jpUp        = 2,
    jpDown      = 3,
};

enum TJoystickMode {
    jm4Way,
    jm8Way
};

class TJoystickToPortHandler
{
public:
    TJoystickToPortHandler( unsigned device, int type, TJoystickMode mode );

    TJoystickToPortHandler( unsigned device, int type, TJoystickMode mode, unsigned char * port, unsigned masks );

    /** Destructor. */
    virtual ~TJoystickToPortHandler();

    void setPort( int index, unsigned char * port, unsigned char and_mask, unsigned char or_mask = 0 );

    bool handleInputEvent( unsigned device, unsigned param );

    void setMode( TJoystickMode mode ) {
        mode_ = mode;
    }

protected:
    unsigned char * port( int index ) const {
        return port_[index];
    }

    unsigned char and_mask( int index ) const {
        return and_mask_[index];
    }

    unsigned char or_mask( int index ) const {
        return or_mask_[index];
    }

    int xpos() const {
        return xpos_;
    }

    int ypos() const {
        return ypos_;
    }

    void setXPos( int pos ) {
        xpos_ = pos;
    }

    void setYPos( int pos ) {
        ypos_ = pos;
    }

    void setDirection( int direction1, int direction2 );

    void setPortAndMasks( unsigned char * port, unsigned masks );

private:
    void initialize( unsigned device, int type, TJoystickMode mode );

    unsigned device_;
    int type_;
    TJoystickMode mode_;
    int xpos_;
    int ypos_;
    unsigned char * port_[4];
    unsigned char and_mask_[4];
    unsigned char or_mask_[4];
};

#endif // EMU_JOYSTICK2PORT_H_
