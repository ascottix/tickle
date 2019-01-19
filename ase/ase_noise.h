/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef ASE_NOISE_H_
#define ASE_NOISE_H_

#include "ase.h"

class AWhiteNoise : public AChannel
{
public:
    AWhiteNoise();

    AWhiteNoise( AFloat frequency );

    void setValue( unsigned value );

    void setTapMask( unsigned tap );

    void setOutput( AFloat hi, AFloat lo );

protected:
    virtual void updateBuffer( AFloat * buf, unsigned len, unsigned ofs );

private:
    unsigned value_;
    unsigned tap_mask_;
    AFloat o_hi_;
    AFloat o_lo_;
    AFloat t_;
    AFloat t_step_;
    AFloat t_half_period_;
    AFloat output_;
};

#endif // ASE_NOISE_H_
