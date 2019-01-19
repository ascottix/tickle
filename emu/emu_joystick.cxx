/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "emu_joystick.h"

TJoystick::TJoystick( unsigned id ) : id_(id) 
{
    x_ = 0;
    y_ = 0;
    dead_area_lo_ = -50;
    dead_area_hi_ = +50;
    buttons_ = 0;
    last_x_ = 0;
    last_y_ = 0;
    last_buttons_ = 0;
    equivalent_keys_state_ = 0;
    for( int i=0; i<MaxButtons; i++ ) {
        button2key_[i] = 0;
    }
}

void TJoystick::setPosition( int x, int y ) 
{
    if( TMath::inRange( x, dead_area_lo_, dead_area_hi_ ) ) {
        x = 0;
    }

    if( TMath::inRange( y, dead_area_lo_, dead_area_hi_ ) ) {
        y = 0;
    }

    x_ = TMath::clip( x, jpMinPos, jpMaxPos );
    y_ = TMath::clip( y, jpMinPos, jpMaxPos );
}

void TJoystick::handleArrowKey( int key, unsigned pressed ) 
{
    // Bit usage:
    // 0x01 = up pressed/released
    // 0x02 = down pressed/released
    // 0x04 = 0 if up was pressed last, 1 if down (used when both keys are pressed)
    // 0x10 = left pressed/released
    // 0x20 = right pressed/released
    // 0x40 = 0 if left was pressed last, 1 if right (used when both keys are pressed)
    switch( key ) {
    case KeyUp:
        equivalent_keys_state_ &= ~0x01;
        if( pressed ) {
            equivalent_keys_state_ &= ~0x04;
            equivalent_keys_state_ |= 0x01;
        }
        break;
    case KeyDown:
        equivalent_keys_state_ &= ~0x02;
        if( pressed ) {
            equivalent_keys_state_ |= 0x06;
        }
        break;
    case KeyLeft:
        equivalent_keys_state_ &= ~0x10;
        if( pressed ) {
            equivalent_keys_state_ &= ~0x40;
            equivalent_keys_state_ |= 0x10;
        }
        break;
    case KeyRight:
        equivalent_keys_state_ &= ~0x20;
        if( pressed ) {
            equivalent_keys_state_ |= 0x60;
        }
        break;
    }
}

void TJoystick::notify( TMachine * machine ) 
{
    if( machine != 0 ) {
        int x = x_;
        int y = y_;

        // If arrow keys are pressed, they override the joystick position
        if( equivalent_keys_state_ & 0x33 ) {
            switch( equivalent_keys_state_ & 0x03 ) {
            case 0x00: y = 0; break;
            case 0x01: y = jpMinPos; break;
            case 0x02: y = jpMaxPos; break;
            case 0x03: y = (equivalent_keys_state_ & 0x04) ? jpMaxPos : jpMinPos; break;
            }

            switch( equivalent_keys_state_ & 0x30 ) {
            case 0x00: x = 0; break;
            case 0x10: x = jpMinPos; break;
            case 0x20: x = jpMaxPos; break;
            case 0x30: x = (equivalent_keys_state_ & 0x40) ? jpMaxPos : jpMinPos; break;
            }
        }

        // Notify position if changed
        if( (x != last_x_) || (y != last_y_) ) {
            last_x_ = x;
            last_y_ = y;
            machine->handleInputEvent( id_, TInput::makeParamFromPos(x,y), 0 );
        }

        // Notify buttons if changed
        if( last_buttons_ != buttons_ ) {
            for( int i=0; i<MaxButtons; i++ ) {
                unsigned bit = 1 << i;

                if( (last_buttons_ & bit) != (buttons_ & bit) ) {
                    unsigned param = (buttons_ & bit) ? 1 : 0;

                    machine->handleInputEvent( button2key_[i], param, 0 );
                }
            }

            last_buttons_ = buttons_;
        }
    }
}
