/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
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
