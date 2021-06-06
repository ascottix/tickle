/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
*/
#ifndef ASE_TRIANGLE_WAVE_VCO1_H_
#define ASE_TRIANGLE_WAVE_VCO1_H_

#include "ase.h"

struct ATriangleWaveVCO
{
    ATriangleWaveVCO( double rampTime1, double rampTime2, double vcoControl );
    
    void play( double toTime = 0, double fadeTime = 0 );
    
    void fade( double fadeTime );
    
    void stop();
    
    void mix( int * buf, int len );
    
    bool stopped() {
        return stopped_;
    }
    
    double wf1_period_[2];
    int wf1_period_index_;
    double gain_;
    double vco_control_;
    int o_;
    int p_;
    double step_;
    double vout_[2]; // v[0] from the "source" triangle wave generator, v[1] from the VCO generator (controlled by first triangle wave)
    bool asc_;
    int stopCount_;
    int fadeCount_;
    int fadeOffset_;
    bool stopped_;
};

#endif // ASE_TRIANGLE_WAVE_VCO1_H_
