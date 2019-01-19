/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef ASE_TIMER555_ASTABLE_H_
#define ASE_TIMER555_ASTABLE_H_

#include "ase.h"

/*
    555 timer in the typical astable configuration.

    Vcc ->-+---------+--+
           |         |  |
           Ra        |  |
           |     +---+--+--+
           +-----|7  8  4  |
           |     |        3|-->- Vout
           Rb    |  TIMER  | 
           |     |   555   |
           +---+-|6        |  
           |   | |        5|-<-- Vcontrol (*)
           C   +-|2  1     | 
           |     +---+-----+
           |         |
    GND ---+---------+

    (*) if control line not specified, 5 is grounded by default.

*/
class ATimer555Astable : public AChannel
{
public:
    ATimer555Astable( AFloat ra, AFloat rb, AFloat c );

    void setRa( AFloat ra );

    void setVcc( AFloat vcc );

    void setVin( AFloat vin );

    /**
        Resets the device.

        This is equivalent to pin 4, only that here the function is abstracted
        with a boolean whereas in the real device you have to pull the pin to
        ground to reset. (In fact, to avoid false resets, this pin is usually
        connected directly to Vcc.)

        The device stays in the specified state until this function is called
        again. Note that output is always low in the reset state.

        @param reset true to reset the device, false for normal operation
    */
    void setReset( bool rest );

    void setControl( AChannel * control, AFloat level );

    void setControl( AChannel * control, AFloat r1, AFloat r2 ) {
        setControl( control, r2 / (r1+r2) );
    }
    
    static double getFrequencyInHertz( AFloat ra, AFloat rb, AFloat c ) {
        return 1 / (0.693 * c * (ra + 2*rb));
    }

protected:
    virtual void updateBuffer( AFloat * buf, unsigned len, unsigned ofs );

private:
    void setCapacitorCoefficients();

    AFloat ra_;
    AFloat rb_;
    AFloat c_;
    AFloat vcc_;
    AFloat vin_;
    AFloat out_hi_;
    AFloat out_lo_;
    AFloat threshold_hi_;
    AFloat threshold_lo_;

    AChannel * control_;
    AFloat control_level_;

    int flipflop_;
    bool reset_;

    AFloat c_voltage_;  // Voltage across capacitor

    AFloat a0_;
    AFloat b0_;
    AFloat a1_;
    AFloat b1_;
};

#endif // ASE_TIMER555_ASTABLE_H_
