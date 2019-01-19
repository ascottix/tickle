/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef ASE_FILTER_H_
#define ASE_FILTER_H_

#include "ase.h"

class AFilter : public AChannel
{
public:
    AFilter( AChannel & source ) : source_(source) {
    }

protected:
    AChannel & source() {
        return source_;
    }

private:
    AChannel &  source_;
};

#endif // ASE_FILTER_H_
