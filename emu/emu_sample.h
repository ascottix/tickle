/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef EMU_SAMPLE_H_
#define EMU_SAMPLE_H_

#include "emu_iostream.h"

class TSample
{
public:
    /** Destructor. */
    virtual ~TSample();

    unsigned size() const {
        return size_;
    }

    unsigned samplingRate() const {
        return sampling_rate_;
    }

    const int * data() const {
        return data_;
    }

    static TSample * createFromWave( TInputStream * is );

private:
    TSample( int * data, unsigned size, unsigned sampling_rate );

    unsigned size_;
    unsigned sampling_rate_;
    int * data_;
};

#endif // EMU_SAMPLE_H_
