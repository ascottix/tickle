/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
*/
#include "ase_multiplexer.h"

AMultiplexer::AMultiplexer()
{
    channel_count_ = 0;
}

void AMultiplexer::addChannel( AChannel * channel, AFloat level )
{
    if( channel_count_ < MaxChannels ) {
        channel_[ channel_count_ ] = channel;
        channel_level_[ channel_count_ ] = level;

        channel_count_++;
    }
}

void AMultiplexer::updateBuffer( AFloat * buf, unsigned len, unsigned ofs )
{
    if( channel_count_ > 0 ) {
        int i;

        // Update all channels
        for( i=0; i<channel_count_; i++ ) {
            channel_[i]->updateTo( ofs );
        }

        // Copy first channel into buffer
        unsigned n;
        AFloat * src = channel_[0]->stream() + streamSize();
        AFloat * dst = buf;

        for( n=0; n<len; n++ ) {
            *dst++ = channel_level_[0] * (*src++);
        }

        // Mix all other channels
        for( i=1; i<channel_count_; i++ ) {
            AFloat * src = channel_[i]->stream() + streamSize();
            AFloat * dst = buf;

            for( n=0; n<len; n++ ) {
                *dst++ += channel_level_[i] * (*src++);
            }
        }
    }
}
