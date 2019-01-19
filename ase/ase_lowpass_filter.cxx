/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "ase_lowpass_filter.h"

ALowPassRCFilter::ALowPassRCFilter( AChannel & source, AFloat r, AFloat c )
    : AFilter( source )
{
    r_ = r;
    c_ = c;

    AFloat d = r_ * c_ * ASE::samplingRate;

    b_ = exp( -1.0 / d );
    a_ = 1.0 - b_;

    y_ = 0;
}

void ALowPassRCFilter::updateBuffer( AFloat * buf, unsigned len, unsigned ofs )
{
    source().updateTo( ofs );

    AFloat * src = source().stream() + streamSize();

    while( len > 0 ) {
        y_ = a_ * (*src++) + b_ * y_;

        *buf++ = y_;

        len--;
    }
}
