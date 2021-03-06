/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
*/
#include "ase_clipper.h"

AClipperLo::AClipperLo( AChannel & source, AFloat lo )
    : AFilter( source )
{
    lo_ = lo;
}

void AClipperLo::updateBuffer( AFloat * buf, unsigned len, unsigned ofs )
{
    source().updateTo( ofs );

    AFloat * src = source().stream() + streamSize();

    while( len > 0 ) {
        if( *src >= lo_ ) {
            *buf = *src;
        }
        else {
            *buf = 0;
        }

        src++;

        buf++;

        len--;
    }
}
