/*
    Namco 51xx custom chip emulator

    Copyright (c) 2011 Alessandro Scotti
*/
#include "namco51.h"

enum {
    ModeNop = 0,
    ModeSetCoinage = 1,
    ModeReadCredits = 2,
    ModeJoystickRemapOff = 3,
    ModeJoystickRemapOn = 4,
    ModeReadPorts = 5
};

Namco51xx::Namco51xx(  unsigned char * port0, unsigned char * port1 )
{
    port0_ = port0;
    port1_ = port1;
    reset();
}
    
void Namco51xx::reset()
{
    credits_ = 0;
    coins_[0] = 0;
    coins_[1] = 0;
    creds_per_coin_[0] = 1;
    creds_per_coin_[1] = 1;
    coins_per_cred_[0] = 1;
    coins_per_cred_[1] = 1;
    mode_ = ModeNop;
    data_count_ = 0;
    o_port0_ = 0xFF;
}

unsigned char Namco51xx::read()
{
    unsigned char b = 0;
    
    if( mode_ == ModeReadPorts ) {
        switch( data_count_ ) {
        case 0:
            b = *port0_;
            break;
        case 1:
            b = *port1_;
            break;
        case 2:
            b = 0;
            break;
        }
    }
    else if( mode_ == ModeReadCredits ) {
        unsigned char v_port0 = *port0_;
        unsigned char v_port1 = *port1_;
        unsigned char toggled_on = (v_port0 ^ o_port0_)  & ~v_port0;
    
        switch( data_count_ ) {
        case 0: // Handle coinage and return number of credits in BCD
            if( coins_per_cred_[0] > 0 ) {
                if( credits_ < 99 ) {
                    if( toggled_on & 0x10 ) { // Coin into coin slot #1
                        coins_[0]++;
                        if( coins_[0] >= coins_per_cred_[0] ) {
                            coins_[0] -= coins_per_cred_[0];
                            credits_ += creds_per_coin_[0];
                        }
                    }
                    
                    if( toggled_on & 0x20 ) { // Coin into coin slot #2
                        coins_[1]++;
                        if( coins_[1] >= coins_per_cred_[1] ) {
                            coins_[1] -= coins_per_cred_[1];
                            credits_ += creds_per_coin_[1];
                        }
                    }
                    
                    if( toggled_on & 0x40 ) { // Service button
                        credits_++;
                    }
                }
            }
            else {
                // Free play
                credits_ = 100;
            }
            
            if( (toggled_on & 0x04) && (credits_ >= 1) ) { // Player 1 start
                credits_ -= 1;
            }
            else if( (toggled_on & 0x08) && (credits_ >= 2) ) { // Player 2 start
                credits_ -= 2;
            }
            
            if( (v_port0 & 0x80) == 0 ) {
                b = 0xBB; // Test mode switch
            }
            else {
                b = (credits_ / 10) * 16 + credits_ % 10;
            }
            break;
        case 1:
            b = v_port1 & 0x0F;
            b |= ((v_port0 | ~o_port0_) & 1) << 4; // Fire event (0 = fire button pressed _and_ it was not pressed last time)
            b |= (v_port0 & 1) << 5; // Fire (0 = pressed)
            break;
        case 2:
            b = v_port1 >> 4;
            b |= ((v_port0 | ~o_port0_) & 2) << 3;
            b |= (v_port0 & 2) << 4;
            
            // Save current value of port 0
            o_port0_ = v_port0;
            break;
        }
    }
    
    data_count_ = (data_count_ + 1) % 3;
    
    return b;
}

void Namco51xx::write( unsigned char b )
{
    b &= 0x07;
    
    if( mode_ == ModeSetCoinage && data_count_ > 0 ) {
        switch( data_count_-- ) {
        case 4: coins_per_cred_[0] = b; break;
        case 3: creds_per_coin_[0] = b; break;
        case 2: coins_per_cred_[1] = b; break;
        case 1: creds_per_coin_[1] = b; break;
        }
    }
    else {
        data_count_ = 0;
        
        switch( b ) {
        case ModeNop:
        case ModeJoystickRemapOff:
        case ModeJoystickRemapOn:
            break;
        case ModeSetCoinage:
            credits_ = 0;
            data_count_ = 4;
            break;
        case ModeReadCredits:
        case ModeReadPorts:
            mode_ = b;
            break;
        }
    }
}
