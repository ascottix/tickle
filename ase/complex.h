/*
    Complex numbers

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef COMPLEX_H_
#define COMPLEX_H_

struct Complex
{
    Complex() {
        real = 0.0;
        imag = 0.0;
    }

    Complex( double r, double i = 0.0 ) {
        real = r;
        imag = i;
    }

    double real;
    double imag;
};

Complex operator * ( const Complex & c1, const Complex & c2 );

Complex operator * ( double a, const Complex & c1 ) ;

Complex operator / ( const Complex & c1, const Complex & c2 );

Complex operator + ( const Complex & c1, const Complex & c2 );

Complex operator - ( const Complex & c1, const Complex & c2 );

Complex operator - ( const Complex & c1 ) ;

Complex csqrt( const Complex & c1 );

#endif // COMPLEX_H_
