/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef EMU_MATH_H_
#define EMU_MATH_H_

// Get rid of annoying macros that may be defined elsewhere
#undef min
#undef max

class TMath
{
public:
    static int abs( int n ) {
        return (n >= 0) ? n : -n;
    }

    static int min( int a, int b ) {
        return (a < b) ? a : b;
    }

    static int max( int a, int b ) {
        return (a > b) ? a : b;
    }

    static int clip( int value, int lo, int hi ) {
        return (value < lo) ? lo : (value > hi) ? hi : value;
    }

    static bool inRange( int value, int lo, int hi ) {
        return (value >= lo) && (value <= hi);
    }

private:
    TMath();
};

#endif // EMU_MATH_H_
