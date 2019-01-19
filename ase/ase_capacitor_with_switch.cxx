/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "ase_capacitor_with_switch.h"

const AFloat InputOnThreshold   = 1.0;
const AFloat ControlOnThreshold = 1.0;

ACapacitorWithSwitch ::ACapacitorWithSwitch( AChannel & input, AChannel & control, AFloat cr, AFloat dr, AFloat c )
    : input_(input), control_(control)
{
    c_ = c;
    r0_ = cr;
    r1_ = dr;

    y_ = 0; // No charge in the capacitor yet

    b0_ = ASE::getRCFactor( cr, c ); // Charge path
    a0_ = 1 - b0_;
    b1_ = ASE::getRCFactor( dr, c ); // Discharge path
}

void ACapacitorWithSwitch ::updateBuffer( AFloat * buf, unsigned len, unsigned ofs )
{
    input_.updateTo( ofs );
    control_.updateTo( ofs );

    AFloat * inp = input_.stream() + streamSize();
    AFloat * ctl = control_.stream() + streamSize();

    while( len > 0 ) {
        // Charge (avoid discharging thru this resistor)
        if( *inp > 0.1 ) {
            y_ = a0_ * (*inp) + b0_ * y_;
        }

        // Discharge and output
        if( *ctl > ControlOnThreshold ) {
            *buf = y_;

            y_ = y_ * b1_;
        }
        else {
            *buf = 0;
        }

        inp++;
        ctl++;

        buf++;

        len--;
    }
}
