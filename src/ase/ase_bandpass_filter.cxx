/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
*/
#include <assert.h>
#include <math.h>

#include "ase_bandpass_filter.h"

const double PI = 3.1415926535898;

AButterworthBandPassFilter::AButterworthBandPassFilter( AChannel & source )
    : AFilter( source )
{
    clear();
}

void AButterworthBandPassFilter::clear()
{
    for( int i=0; i<5; i++ ) {
        x[i] = 0.0;
        y[i] = 0.0;
    }
}

void AButterworthBandPassFilter::setBand( double lo_freq, double hi_freq )
{
    Complex s_pole[4];
    Complex z_pole[4];
    Complex z_zero[4];

    int i;

    // Scale frequencies to sampling rate
    lo_freq /= ASE::samplingRate;
    hi_freq /= ASE::samplingRate;

    // Initialize s-plane (4 poles for order 2, but only two are created here)
    int poles = 0;

    for( i=0; i<4; i++ ) {
        double t = ((i+0.5)*PI) / 2;
        Complex c( cos(t), sin(t) );

        if( c.real < 0.0 ) {
            s_pole[ poles++ ] = c;
        }
    }

    assert( poles == 2 );

    // Pre-warp frequencies
    double warped_lo_freq = tan( PI*lo_freq ) / PI;
    double warped_hi_freq = tan( PI*hi_freq ) / PI;

    // Normalize
    double w_lo = 2 * PI * warped_lo_freq;
    double w_hi = 2 * PI * warped_hi_freq;
    double w = sqrt(w_lo*w_hi);
    double bw = w_hi - w_lo;

    // Create remaining poles
    for( i=0; i<2; i++ ) { 
        Complex hba = 0.5 * (s_pole[i] * bw);
        Complex temp = csqrt( 1.0 - (w/hba)*(w/hba) );

        s_pole[i] = hba * (1.0 + temp);
        s_pole[i+2] = hba * (1.0 - temp);
    }

    // Bilinear transform from s-plane to z-plane
    for( i=0; i<4; i++ ) {
        z_pole[i] = Complex( (2.0 + s_pole[i]) / (2.0 - s_pole[i]) );
        z_zero[i] = (i < 2) ? Complex( +1.0 ) : Complex( -1.0 );
    }

    // Compute polynomials in z
    Complex a_coeff[5] = { 1.0, 0.0, 0.0, 0.0, 0.0 };
    Complex b_coeff[5] = { 1.0, 0.0, 0.0, 0.0, 0.0 };

    for( i=0; i<4; i++ ) {
        int j;
        Complex c;
        
        c = - z_zero[i];

        for( j=4; j>=1; j-- ) {
            a_coeff[j] = (c * a_coeff[j]) + a_coeff[j-1];
        }

        a_coeff[0] = c * a_coeff[0];

        c = - z_pole[i];

        for( j=4; j>=1; j-- ) {
            b_coeff[j] = (c * b_coeff[j]) + b_coeff[j-1];
        }

        b_coeff[0] = c * b_coeff[0];
    }

    // Compute coefficients
    for( i=0; i<=4; i++ ) {
        a[i] = (AFloat) +(a_coeff[i].real / b_coeff[4].real);
        b[i] = (AFloat) -(b_coeff[i].real / b_coeff[4].real);
    }

    // Set gain
    gain_ = 1.0;
}

void AButterworthBandPassFilter::updateBuffer( AFloat * buf, unsigned len, unsigned ofs )
{
    source().updateTo( ofs );

    AFloat * src = source().stream() + streamSize();

    while( len > 0 ) {
        x[0] = x[1]; 
        x[1] = x[2]; 
        x[2] = x[3]; 
        x[3] = x[4]; 

        x[4] = *src++ / gain_;

        y[0] = y[1]; 
        y[1] = y[2]; 
        y[2] = y[3]; 
        y[3] = y[4]; 

        y[4] = a[0]*x[0] + a[1]*x[1] + a[2]*x[2] + a[3]*x[3] + a[4]*x[4] +
               b[0]*y[0] + b[1]*y[1] + b[2]*y[2] + b[3]*y[3];

        *buf++ = y[4];

        len--;
    }
}

AActiveBandPassFilter::AActiveBandPassFilter( AChannel & source, double r1, double r2, double r3, double c1, double c2 )
    : AButterworthBandPassFilter( source )
{
    r1_ = r1;
    r2_ = r2;
    r3_ = r3;
    c1_ = c1;
    c2_ = c2;

    setBand();
}

void AActiveBandPassFilter::setBand()
{
    double w0 = sqrt( (1 / (r3_*c1_*c2_)) * (1/r1_ + 1/r2_) );
    double q0 = w0 / ( (1/r3_) * (1/c1_ + 1/c2_) );
    double f0 = w0 / (2 * PI);
    double bw = f0 / q0;

    /*
        These are more correct values:

        double f_lo = -f0/(2*q0) + (f0*sqrt(1+4*q0*q0)) / (2*q0);
        double f_hi = +f0/(2*q0) + (f0*sqrt(1+4*q0*q0)) / (2*q0);
    */

    double f_lo = f0 - bw / 2;
    double f_hi = f0 + bw / 2;

    AButterworthBandPassFilter::setBand( f_lo, f_hi );
}
