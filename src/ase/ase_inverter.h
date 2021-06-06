/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
*/
#ifndef ASE_INVERTER_H_
#define ASE_INVERTER_H_

#include "ase.h"
#include "ase_filter.h"

class AInverter : public AFilter
{
public:
    AInverter( AChannel & source, AFloat hi, AFloat lo, AFloat threshold = 2.5 );

protected:
    virtual void updateBuffer( AFloat * buf, unsigned len, unsigned ofs );

private:
    AFloat hi_;
    AFloat lo_;
    AFloat threshold_;
};

#endif // ASE_INVERTER_H_
