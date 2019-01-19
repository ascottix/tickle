/*
    AY-3-8910 sound chip emulator

    Copyright (c) 2004-2011 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "ay-3-8910.h"

static int Ay38910_VolumeTable[16] = { 
      0, 2, 3, 6, 10, 16, 25, 37, 52, 72, 95, 122, 152, 185, 219, 255
};

AY_3_8910::AY_3_8910( unsigned clock )
    : YM2149( clock )
{
    // Set proper (as introduced in Tickle 0.94) volume and envelope
    for( int i=0; i<16; i++ ) {
        int v = Ay38910_VolumeTable[i];
        VolumeTable[i] = v;
        EnvelopeVolumeTable[i*2]    = v; // AY-3-8910 has 16 envelope steps
        EnvelopeVolumeTable[i*2+1]  = v;
    }
}
