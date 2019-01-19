/*
    Waveform generation

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef WAVEFORM_H_
#define WAVEFORM_H_

class TSquareWaveGenerator
{
public:
    TSquareWaveGenerator();

    ~TSquareWaveGenerator() {
    }

    void setFrequency( double f_hertz ) {
        setHiLoPeriod( 0.5 / f_hertz );
    }

    /**
        Sets the periods of the square wave.

        Note the values of the "low" and "initial high" periods are optional.
        If omitted, they are assumed to be equal to the "high" period.

        @param t_hi period of "high" state in seconds
        @param t_lo period of "low" state in seconds
        @param t_initial_hi period of "high" state in seconds after reset
    */
    void setHiLoPeriod( double t_hi, double t_lo = 0, double t_initial_hi = 0 );

    void setAmplitude( int hi, int lo = 0 );

    void reset() {
        current_offset_ = 0;
        half_period_index_ = 0;
    }

    void addToBuffer( int * data, unsigned len, unsigned samplingRate ) {
        apply( opAdd, data, len, samplingRate );
    }

    void andWithBuffer( int * data, unsigned len, unsigned samplingRate ) {
        apply( opAnd, data, len, samplingRate );
    }

    void setBuffer( int * data, unsigned len, unsigned samplingRate ) {
        apply( opSet, data, len, samplingRate );
    }

private:
    enum {
        opSet, opAdd, opAnd, opSub
    };

    void apply( int op, int * data, unsigned len, unsigned samplingRate );

    unsigned half_period_samples_[3];
    unsigned half_period_index_;
    unsigned current_offset_;
    unsigned sampling_rate_;
    double half_period_secs_[3];
    int value_lo_;
    int value_hi_;
};

#endif // WAVEFORM_H_
