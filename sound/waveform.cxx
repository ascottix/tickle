/*
    Waveform generation

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "waveform.h"

TSquareWaveGenerator::TSquareWaveGenerator()
{
    setHiLoPeriod( 0 );
    setAmplitude( 1, 0 );
    reset();
}

void TSquareWaveGenerator::setHiLoPeriod( double t_hi, double t_lo, double t_initial_hi )
{
    half_period_secs_[2] = t_hi;
    half_period_secs_[1] = t_lo != 0 ? t_lo : t_hi;
    half_period_secs_[0] = t_initial_hi != 0 ? t_initial_hi : t_hi;
    sampling_rate_ = 0;
}

void TSquareWaveGenerator::setAmplitude( int hi, int lo )
{
    value_lo_ = lo;
    value_hi_ = hi;
}

void TSquareWaveGenerator::apply( int op, int * data, unsigned len, unsigned samplingRate )
{
    if( samplingRate != sampling_rate_ ) {
        // Convert time from seconds to samples (note that we discard decimals here,
        // so there may be errors if period is very small)
        for( int i=0; i<3; i++ ) {
            half_period_samples_[i] = (unsigned) (half_period_secs_[i] * samplingRate);
        }

        sampling_rate_ = samplingRate;
    }

    int value = (half_period_index_ == 1) ? value_lo_ : value_hi_;

    while( len > 0 ) {
        // Get max number of samples with same value
        unsigned count = half_period_samples_[half_period_index_] - current_offset_;

        if( count > len ) {
            count = len;
        }

        // Update counters
        current_offset_ += count;
        len -= count;

        // Apply selected operation to sample block
        switch( op ) {
        case 0:
            // Set
            for( ; count > 0; count-- ) *data++  = value;
            break;
        case 1:
            // Add
            for( ; count > 0; count-- ) *data++ += value;
            break;
        case 2:
            // And
            for( ; count > 0; count-- ) *data++ &= value;
            break;
        case 3:
            // Sub
            for( ; count > 0; count-- ) *data++ -= value;
            break;
        }

        // Invert value if needed
        if( current_offset_ >= half_period_samples_[half_period_index_] ) {
            current_offset_ = 0;

            if( ++half_period_index_ > 2 ) {
                half_period_index_ = 1;
            }

            value = (half_period_index_ == 1) ? value_lo_ : value_hi_;
        }
    }
}
