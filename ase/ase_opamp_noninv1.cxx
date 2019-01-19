/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "ase_opamp_noninv1.h"

AOpAmp_NonInv1::AOpAmp_NonInv1( AChannel & source, AFloat r1, AFloat r2, AFloat r3, AFloat vcc )
    : AFilter( source )
{
    r1_ = r1;
    r2_ = r2;
    r3_ = r3;

    vcc_ = vcc;

    /*
        Voltage at inverting input is:

        Vm = Vcc * (RA / (R1 + RA)) + Vout * (RB / (R3 + RB))

        where RA = R2||R3 and RB = R1||R2.

        Forcing Vin = Vm we get:

        Vin = Vcc * (RA / (R1 + RA)) + Vout * (RB / (R3 + RB))

        Vout = (Vin - Vcc * (RA / (R1 + RA))) / (RB / (R3 + RB))
    */
    AFloat ra = ASE::resInParallel( r2, r3 );
    AFloat rb = ASE::resInParallel( r1, r2 );

    a_ = vcc * (ra / (r1 + ra));
    b_ = rb / (r3 + rb);
}

void AOpAmp_NonInv1::updateBuffer( AFloat * buf, unsigned len, unsigned ofs )
{
    source().updateTo( ofs );

    AFloat * src = source().stream() + streamSize();

    while( len > 0 ) {
        *buf = (*src - a_) / b_;

        src++;

        buf++;

        len--;
    }
}
