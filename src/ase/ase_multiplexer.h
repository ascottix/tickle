/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
*/
#ifndef ASE_MULTIPLEXER_H_
#define ASE_MULTIPLEXER_H_

#include "ase.h"

class AMultiplexer : public AChannel
{
public:
    AMultiplexer();

    void addChannel( AChannel * channel, AFloat level );

protected:
    virtual void updateBuffer( AFloat * buf, unsigned len, unsigned ofs );

private:
    enum {
        MaxChannels = 8
    };

    AChannel * channel_[ MaxChannels ];
    AFloat channel_level_[ MaxChannels ];
    int channel_count_;
};

#endif // ASE_MULTIPLEXER_H_
