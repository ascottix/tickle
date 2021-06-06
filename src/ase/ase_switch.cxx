/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
*/
#include "ase_switch.h"

const AFloat ControlOnThreshold = 0.5;

ASwitch::ASwitch( AChannel & source, AChannel & control )
    : AFilter(source), control_(control)
{
}

void ASwitch::updateBuffer( AFloat * buf, unsigned len, unsigned ofs )
{
    source().updateTo( ofs );
    control_.updateTo( ofs );

    AFloat * src = source().stream() + streamSize();
    AFloat * ctl = control_.stream() + streamSize();

    while( len > 0 ) {
        if( *ctl > ControlOnThreshold ) {
            *buf = *src;
        }
        else {
            *buf = 0;
        }

        buf++;
        ctl++;
        src++;

        len--;
    }
}
