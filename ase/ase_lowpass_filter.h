/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef ASE_LOWPASS_FILTER_H_
#define ASE_LOWPASS_FILTER_H_

#include "ase.h"
#include "ase_filter.h"

class ALowPassRCFilter : public AFilter
{
public:
    ALowPassRCFilter( AChannel & source, AFloat r, AFloat c );

    void setCharge( AFloat y ) {
        y_ = y;
    }

protected:
    virtual void updateBuffer( AFloat * buf, unsigned len, unsigned ofs );

private:
    AFloat r_;
    AFloat c_;
    AFloat a_;
    AFloat b_;
    AFloat y_;
};

#endif // ASE_LOWPASS_FILTER_H_
