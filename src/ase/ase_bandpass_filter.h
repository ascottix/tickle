/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
*/
#ifndef ASE_BANDPASS_FILTER_H_
#define ASE_BANDPASS_FILTER_H_

#include "ase.h"
#include "ase_filter.h"

#include "complex.h"

/*
    2nd order Butterworth bandpass filter.
*/
class AButterworthBandPassFilter : public AFilter
{
public:
    AButterworthBandPassFilter( AChannel & source );

    void clear();

    void setBand( double lo_freq, double hi_freq );

    AFloat gain() {
        return gain_;
    }

    void setGain( AFloat gain ) {
        gain_ = gain;
    }

protected:
    virtual void updateBuffer( AFloat * buf, unsigned len, unsigned ofs );

private:
    AFloat gain_;
    AFloat x[5];
    AFloat y[5];
    AFloat a[5];
    AFloat b[5]; // For simmetry, actually b[0] is never used
};

/*
    Second-order active pass-band filter.
    
    This class implements a filter of the following type:


                +--------+------------+
                |        |            |
                C1      R3   \        |
                |        |   | \      |
    Vin ---R1---+---C2---+---| - \    |
                |            |    >---+--- Vout
                R2       +---| + /
                |        |   | / 
                |        |   /
               GND     Vref  


    As I haven't yet worked out the math to implement the above circuit,
    at present it is remapped to a 2nd-order Butterworth filter.
*/
class AActiveBandPassFilter : public AButterworthBandPassFilter
{
public:
    AActiveBandPassFilter( AChannel & source, double r1, double r2, double r3, double c1, double c2 );

    void setR1( double r1 ) {
        r1_ = r1;
    }

    void setBand();

private:
    double r1_;
    double r2_;
    double r3_;
    double c1_;
    double c2_;
};

#endif // ASE_BANDPASS_FILTER_H_
