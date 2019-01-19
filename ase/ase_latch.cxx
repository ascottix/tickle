/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "ase_latch.h"

ALatch::ALatch( AFloat value )
{
    value_ = value;
}

void ALatch::updateBuffer( AFloat * buf, unsigned len, unsigned ofs )
{
    while( len > 0 ) {
        *buf++ = value_;

        len--;
    }
}
