/*
    RC filter

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef RCFILTER_H_
#define RCFILTER_H_

class RCFilter 
{
public:
    RCFilter( double r, double c ) {
        setRC( r, c );
        y_ = 0.0;
    }

    void setRC( double r, double c ) {
        r_ = r;
        c_ = c;
        sampling_rate_ = 0; // Force a refresh next time filter is applied
    }

    void reset( double y = 0.0 ) {
        y_ = y;
    }

    void apply( int * buf, unsigned len, unsigned samplingRate );

private:
    unsigned sampling_rate_;
    double y_; // Last sample
    double a_;
    double b_;
    double r_; // Resistor value in Ohms
    double c_; // Capacitor value in Farads
};

#endif // RCFILTER_H_
