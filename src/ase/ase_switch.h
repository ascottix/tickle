/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
*/
#ifndef ASE_SWITCH_H_
#define ASE_SWITCH_H_

#include "ase.h"
#include "ase_filter.h"

class ASwitch : public AFilter
{
public:
    ASwitch( AChannel & source, AChannel & control );

protected:
    virtual void updateBuffer( AFloat * buf, unsigned len, unsigned ofs );

private:
    AChannel & control_;
};

#endif // ASE_SWITCH_H_
