/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef EMU_SAMPLE_PLAYER_H_
#define EMU_SAMPLE_PLAYER_H_

#include "emu_mixer.h"
#include "emu_sample.h"

enum {
    pmLooping = 1
};

class TSamplePlayer
{
public:
    TSamplePlayer();

    /** Destructor. */
    virtual ~TSamplePlayer();

    void setSample( TSample * sample );

    void play( unsigned mode = 0 );

    void stop();
    
    void restart() {
        offset_ = 0;
    }

    bool mix( int * buf, unsigned len, unsigned samplingRate );

    bool mix( TMixer * mixer, unsigned channel, unsigned len, unsigned samplingRate );

    TSample * sample() {
        return sample_;
    }

private:
    unsigned status_;
    unsigned offset_;
    TSample * sample_;
};

#endif // EMU_SAMPLE_PLAYER_H_
