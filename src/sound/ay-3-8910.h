/*
    AY-3-8910 sound chip emulator

    Copyright (c) 2004-2011 Alessandro Scotti
*/
#ifndef AY_3_8910_H_
#define AY_3_8910_H_

#include "ym2149.h"

class AY_3_8910 : public YM2149
{
public:
    AY_3_8910( unsigned clock = YM2149::MinClock );
};

#endif // AY_3_8910_H_
