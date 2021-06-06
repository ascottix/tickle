/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
*/
#ifndef ASE_OPAMP_NONINV1_H_
#define ASE_OPAMP_NONINV1_H_

#include "ase.h"
#include "ase_filter.h"

/*
    This class implements a non-inverting operational amplifier
    with the following configuration:

                Vcc
                 |
                 R1
                 |
        +---R2---+-----R3-----+
        |        |            |
        |        |   \        |
        |        |   | \      |
       GND       +---| - \    |
                     |    >---+--- Vout
             Vin +---| + /
                     | / 
                     /
*/
class AOpAmp_NonInv1 : public AFilter
{
public:
    AOpAmp_NonInv1( AChannel & source, AFloat r1, AFloat r2, AFloat r3, AFloat vcc );

protected:
    virtual void updateBuffer( AFloat * buf, unsigned len, unsigned ofs );

private:
    AFloat r1_;
    AFloat r2_;
    AFloat r3_;
    AFloat vcc_;
    AFloat a_;
    AFloat b_;
};

#endif // ASE_OPAMP_NONINV1_H_
