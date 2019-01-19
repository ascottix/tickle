/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef ASE_LATCH_H_
#define ASE_LATCH_H_

#include "ase.h"

class ALatch : public AChannel
{
public:
    ALatch( AFloat value = 0 );

    void setValue( AFloat value ) {
        value_ = value;
    }

protected:
    virtual void updateBuffer( AFloat * buf, unsigned len, unsigned ofs );

private:
    AFloat value_;
};

#endif // ASE_CAPACITOR_H_
