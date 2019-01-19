/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef ASE_CAPACITOR_WITH_SWITCH_H_
#define ASE_CAPACITOR_WITH_SWITCH_H_

#include "ase.h"

/*
    An emulation of the following circuit:

    
    Vin ---R1---C---|X|---R2---+--- Vout
                     |         |
                     |         |
                  Control     GND

    That is, capacitor C is charges thru R1 but only discharges thru R2 when
    the switch is open.
*/
class ACapacitorWithSwitch : public AChannel
{
public:
    ACapacitorWithSwitch( AChannel & input, AChannel & control, AFloat r1, AFloat r2, AFloat c );

    void setCharge( AFloat charge ) {
        y_ = charge;
    }

protected:
    virtual void updateBuffer( AFloat * buf, unsigned len, unsigned ofs );

private:
    AChannel & input_;
    AChannel & control_;
    AFloat c_;
    AFloat r0_;
    AFloat r1_;
    AFloat a0_;
    AFloat b0_;
    AFloat b1_;
    AFloat y_;
};

#endif // ASE_CAPACITOR_WITH_SWITCH_H_
