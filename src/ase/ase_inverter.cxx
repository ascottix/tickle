/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
*/
#include "ase_inverter.h"

AInverter::AInverter( AChannel & source, AFloat hi, AFloat lo, AFloat threshold )
    : AFilter( source )
{
    hi_ = hi;
    lo_ = lo;
    threshold_ = threshold;
}

void AInverter::updateBuffer( AFloat * buf, unsigned len, unsigned ofs )
{
    source().updateTo( ofs );

    AFloat * src = source().stream() + streamSize();

    while( len > 0 ) {
        if( *src++ > threshold_ ) {
            *buf = lo_;
        }
        else {
            *buf = hi_;
        }

        buf++;

        len--;
    }
}
