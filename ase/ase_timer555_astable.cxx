/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "ase_timer555_astable.h"

ATimer555Astable::ATimer555Astable( AFloat ra, AFloat rb, AFloat c )
{
    c_ = c;
    ra_ = ra;
    rb_ = rb;
    out_hi_ = 3.3;
    out_lo_ = 0.2;
    c_voltage_ = 0; // Voltage across capacitor
    flipflop_ = 1;  // Charging
    reset_ = false;
    control_ = 0;
    
    setVcc( 5.0 );
    setVin( vcc_ );
    setCapacitorCoefficients();
}

void ATimer555Astable::setRa( AFloat ra )
{
    ra_ = ra;
    setCapacitorCoefficients();
}

void ATimer555Astable::setVin( AFloat vin )
{
    vin_ = vin;
}

void ATimer555Astable::setVcc( AFloat vcc )
{
    vcc_ = vcc;

    if( control_ == 0 ) {
        // Assume control is grounded
        threshold_lo_ = (AFloat) (vcc_ / 3);
        threshold_hi_ = threshold_lo_ * 2;
    }
}

void ATimer555Astable::setControl( AChannel * control, AFloat level )
{
    control_ = control;
    control_level_ = level;

    setVcc( vcc_ );
}

void ATimer555Astable::setCapacitorCoefficients()
{
    AFloat d;

    // Charging
    d = c_ * (ra_ + rb_) * ASE::samplingRate;
    b0_ = (AFloat) exp( -1.0 / d );
    a0_ = (AFloat) 1.0 - b0_;

    // Discharging
    d = c_ * rb_ * ASE::samplingRate;
    b1_ = (AFloat) exp( -1.0 / d );
    a1_ = (AFloat) 1.0 - a0_;
}

void ATimer555Astable::setReset( bool reset )
{
    reset_ = reset;

    if( reset ) {
        // Discharge capacitor
        c_voltage_ = 0;
        flipflop_ = 1;
    }
}

void ATimer555Astable::updateBuffer( AFloat * buf, unsigned len, unsigned ofs )
{
    // Update control if defined (regardless of whether we're using it or not)
    if( control_ != 0 ) {
        control_->updateTo( ofs );
    }

    if( reset_ ) {
        // If reset, output is always low
        while( len > 0 ) {
            *buf++ = out_lo_;

            len--;
        }
    }
    else if( control_ == 0 ) {
        // Threshold points are fixed (set by resistors)
        while( len > 0 ) {
            if( flipflop_ ) {
                *buf++ = out_hi_;

                // Capacitor is charging
                c_voltage_ = a0_ * vin_ + b0_ * c_voltage_;

                if( c_voltage_ >= threshold_hi_ ) {
                    flipflop_ = 0;
                }
            }
            else {
                *buf++ = out_lo_;

                // Capacitor is discharging
                c_voltage_ = b1_ * c_voltage_;

                if( c_voltage_ <= threshold_lo_ ) {
                    flipflop_ = 1;
                }
            }

            len--;
        }
    }
    else {
        // Threshold points are set externally by control voltage
        AFloat * ctl = control_->stream() + streamSize();

        while( len > 0 ) {
            AFloat level = *ctl++ * control_level_;

            if( flipflop_ ) {
                *buf++ = out_hi_;

                // Capacitor is charging
                c_voltage_ = a0_ * vin_ + b0_ * c_voltage_;

                if( c_voltage_ >= level ) {
                    flipflop_ = 0;
                }
            }
            else {
                *buf++ = out_lo_;

                // Capacitor is discharging
                c_voltage_ = b1_ * c_voltage_;

                if( c_voltage_ <= (level / 2) ) {
                    flipflop_ = 1;
                }
            }

            len--;
        }
    }
}
