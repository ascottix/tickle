/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef ASE_CLIPPER_H_
#define ASE_CLIPPER_H_

#include "ase.h"
#include "ase_filter.h"

class AClipperLo : public AFilter
{
public:
    AClipperLo( AChannel & source, AFloat lo );

protected:
    virtual void updateBuffer( AFloat * buf, unsigned len, unsigned ofs );

private:
    AFloat lo_;
};

#endif // ASE_CLIPPER_H_
