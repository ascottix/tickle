/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "ase_triangle_wave_vco1.h"

/*
    This class implements the voltage controlled oscillator that generates the
    ufo hit and target hit sounds in Space Invaders.
    
    The circuit includes two oscillators, each based on a LM3900 op-amp.
    
    The first oscillator generates a triangle wave of fixed frequency and shape,
    which is controlled by a couple of resistors in the circuit. These parameters
    are passed to the class constructor as the "ramp time" for the ascending and
    descending parts of the triangle wave, from which both frequency and shape
    follow. The output is between 1.2 and 11 volts.
    
    The second oscillator generates a triangle wave whose frequency is controlled
    by the output of the first oscillator. Higher output values yield faster
    frequencies. The output is between 0.5 and 9 volts.
    
    It is possible to have the output stop automatically after a specified time,
    which helps emulate a "one-shot amplifier" circuit used by the "target hit" sound, 
    also based on a LM3900.
    
    In order to obtain values for the above parameters, including the correlation
    between the output of the first oscillator and the frequency of the second,
    I have used a "Spice" program to simulate the circuits, followed by a semi-manual
    regression to correlate the output values of the two oscillators.
*/
ATriangleWaveVCO::ATriangleWaveVCO( double rampTime1, double rampTime2, double vcoControl )
{
    wf1_period_[0] = rampTime1 * ASE::samplesPerMs;
    wf1_period_[1] = rampTime2 * ASE::samplesPerMs;
    
    vco_control_ = vcoControl;
    
    gain_ = 10;
    
    stop();
}

void ATriangleWaveVCO::play( double stopTime, double fadeTime )
{
    stopped_ = false;
    
    stopCount_ = (int) ((AFloat) ASE::samplingRate * stopTime);
    fadeCount_ = (int) ((AFloat) ASE::samplingRate * fadeTime);
    fadeOffset_ = 0;
}

void ATriangleWaveVCO::fade( double fadeTime )
{
    stopCount_ = 0;
    fadeCount_ = (int) ((AFloat) ASE::samplingRate * fadeTime);
    fadeOffset_ = 0;
}

void ATriangleWaveVCO::stop()
{
    stopped_ = true;
    stopCount_ = 0;
    
    vout_[0] = 1.2; // Min value of first oscillator output
    vout_[1] = 0.5; // Min value of second (VCO) oscillator output
    
    wf1_period_index_ = 0;
    o_ = 0;
    p_ = wf1_period_[ wf1_period_index_ ];
    step_ = 9.8 / p_; // Delta value of first oscillator output (min=1.2, max=11.0)
    asc_ = true;
}

void ATriangleWaveVCO::mix( int * buf, int len )
{
    while( len > 0 ) {
        len--;
        
        double gain = gain_;

        if( stopCount_ > 0 ) {
            stopCount_--;
            if( stopCount_ == 0  && fadeCount_ == 0 ) stop();
        }
        else if( ! stopped_ && fadeCount_ > 0 ) {
            fadeOffset_++;
            gain *= exp( -( (double)fadeOffset_ * 3.0) / ((double) fadeCount_) );
            if( fadeOffset_ == fadeCount_ ) stop();
        }
        
        if( stopped_ ) {
            *buf++ += 0;
            continue;
        }
        else {
            *buf++ += (int) (vout_[1] * gain);
        }
        
        // Update second (VCO) oscillator
        double p2 = ASE::samplesPerMs * (vco_control_ / pow(vout_[0], 1.1 )); // Obtained by simulation and then regression
        double s2 = 8.5 / p2; // Step, 8.5 is the amplitude of the second oscillator output (min=0.5, max=9.0)
        
        if( asc_ ) { // Ascending
            vout_[1] += s2;
            if( vout_[1] >= 9.0 ) asc_ = ! asc_;
        }
        else {
            vout_[1] -= s2;
            if( vout_[1] <= 0.5 ) asc_ = ! asc_;
        }
        
        // Update first oscillator
        vout_[0] += step_;
        
        o_++;
        if( o_ >= p_ ) {
            wf1_period_index_ ^= 1;
            p_ = (int) wf1_period_[ wf1_period_index_ ];
            step_ = 9.8 / p_; 
            if( wf1_period_index_ ) {
                step_ = -step_;
            }
            else {
                vout_[0] = 1.2; // Reset to min out value
            }
            o_ = 0;
        }
    }
}
