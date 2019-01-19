/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include <string.h>

#include "emu_math.h"
#include "emu_mixer.h"

void TMixerBuffer::clear()
{
    if( data_ != 0 ) {
        memset( data_, 0, sizeof(int)*size_ );
    }

    num_voices_ = 0;
}

void TMixerBuffer::expand( unsigned size )
{
    if( size_ < size ) {
        int * data = new int [ size ];

        if( data_ != 0 ) {
            memcpy( data, data_, sizeof(int)*size_ );
            memset( data + sizeof(int)*size_, 0, sizeof(int)*(size-size_) );
            delete data_;
        }
        else {
            memset( data, 0, sizeof(int)*size );
        }

        data_ = data;
        size_ = size;
    }
}

TMixer::TMixer( unsigned channels )
{
    num_channels_ = channels;
    buffers_ = new TMixerBuffer[ channels ];
}

TMixer::~TMixer()
{
    delete [] buffers_;
}

void TMixer::clear()
{
    for( unsigned i=0; i<num_channels_; i++ ) {
        buffers_[i].clear();
    }
}

int TMixer::maxVoicesPerChannel()
{
    int result = 0;

    for( unsigned i=1; i<num_channels_; i++ ) {
        result = TMath::max( result, buffers_[i].voices() );
    }

    return result + buffers_[0].voices();
}

TMixerBuffer * TMixer::getBuffer( unsigned channel, unsigned length, unsigned voices )
{
    TMixerBuffer * result = 0;

    if( channel < num_channels_ ) {
        result = &buffers_[channel];

        result->expand( length );
        result->addVoices( voices );
    }

    return result;
}

TMixerBuffer * TMixerMono::getBuffer( unsigned channel, unsigned length, unsigned voices ) 
{
    return TMixer::getBuffer( 0, length, voices );
}

TMixerBuffer * TMixerStereo::getBuffer( unsigned channel, unsigned length, unsigned voices )
{
    return TMixer::getBuffer( (channel >= 2) ? 0 : channel, length, voices );
}
