/*
    Complex numbers

    Copyright (c) 2004 Alessandro Scotti
*/
#include <math.h>

#include "complex.h"

Complex operator * ( const Complex & c1, const Complex & c2 )
{
    return Complex( c1.real*c2.real - c1.imag*c2.imag, c1.real*c2.imag + c1.imag*c2.real );
}

Complex operator * ( double a, const Complex & c1 ) 
{
    return Complex( a*c1.real, a*c1.imag );
}

Complex operator / ( const Complex & c1, const Complex & c2 )
{ 
    double m = (c2.real * c2.real) + (c2.imag * c2.imag);
    double r = ((c1.real * c2.real) + (c1.imag * c2.imag)) / m;
    double i = ((c1.imag * c2.real) - (c1.real * c2.imag)) / m;

    return Complex( r, i );
}

Complex operator + ( const Complex & c1, const Complex & c2 )
{
    return Complex( c1.real+c2.real, c1.imag+c2.imag );
}

Complex operator - ( const Complex & c1, const Complex & c2 )
{
    return Complex( c1.real-c2.real, c1.imag-c2.imag );
}

Complex operator - ( const Complex & c1 ) 
{
    return Complex( -c1.real, -c1.imag );
}

Complex csqrt( const Complex & c1 )
{ 
    double r = hypot( c1.imag, c1.real );

    Complex result( sqrt(0.5 * (r + c1.real)), sqrt(0.5 * (r - c1.real)) );

    if( c1.imag < 0.0 ) {
        result.imag = -result.imag;
    }

    return result;
}
