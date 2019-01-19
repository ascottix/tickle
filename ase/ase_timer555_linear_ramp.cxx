/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "ase_timer555_linear_ramp.h"

ATimer555LinearRamp::ATimer555LinearRamp( AFloat c )
{
    c_ = c;
    c_voltage_ = 0; // Initial charge
    flipflop_ = 1;  // Charging
    current_ = 0;
    c_dt_ = c * ASE::samplingRate;
    
    setVcc( 5.0 );
}

void ATimer555LinearRamp::setVcc( AFloat vcc )
{
    vcc_ = vcc;
    threshold_lo_ = (AFloat) (vcc_ / 3);
    threshold_hi_ = threshold_lo_ * 2;
}

void ATimer555LinearRamp::setCurrent( AChannel * current )
{
    current_ = current;
}

void ATimer555LinearRamp::updateBuffer( AFloat * buf, unsigned len, unsigned ofs )
{
    assert( current_ != 0 );

    current_->updateTo( ofs );

    AFloat * ctl = current_->stream() + streamSize();

    while( len > 0 ) {
        *buf++ = c_voltage_;

        AFloat s = *ctl++ / c_dt_;

        // Should check for zero voltage: for now, this check
        // must be performed somewhere else (i.e. when setting the
        // controlling current)
        c_voltage_ += s;

        if( c_voltage_ >= threshold_hi_ ) {
            // For practical purposes, we can assume voltage drop instantaneously
            // (actually it will take 25-30 microseconds or so)
            c_voltage_ = threshold_lo_;
        }

        len--;
    }
}
