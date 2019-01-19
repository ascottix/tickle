/*
    RC filter

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include <math.h>

#include "rcfilter.h"

void RCFilter::apply( int * buf, unsigned len, unsigned samplingRate ) 
{
    // Recalc coefficients if needed
    if( samplingRate != sampling_rate_ ) {
        sampling_rate_ = samplingRate;

        double d = r_ * c_ * sampling_rate_;
        
        b_ = exp( -1/d );
        a_ = 1 - b_;
    }

    while( len > 0 ) {
        y_ = a_ * (*buf) + b_ * y_;
        *buf++ = (int) y_;
        len--;
    }
}
