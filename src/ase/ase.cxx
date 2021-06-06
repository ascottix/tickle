/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
*/
#include <string.h>

#include "ase.h"

#ifdef WIN32

#include <stdio.h>
#include <stdlib.h>

void trace( const char * fmt, ... )
{
    va_list vl;

    va_start( vl, fmt );

    char buf[1000];

    vsprintf( buf, fmt, vl );

    OutputDebugString( buf );

    va_end( vl );
}

#endif // WIN32

const unsigned SamplingRate = 44100;

unsigned ASE::samplingRate = SamplingRate;

AFloat ASE::samplesPerMs = ((AFloat) SamplingRate) / 1000.0;

unsigned ASE::initialBufferSize = 0;

AChannel::AChannel()
{
    if( ASE::initialBufferSize == 0 ) {
        buffer_ = 0;
        buflen_ = 0;
    }
    else {
        buffer_ = new AFloat [ ASE::initialBufferSize ];
        buflen_ = ASE::initialBufferSize;
    }

    bufofs_ = 0;

    enabled_ = true;
}

AChannel::~AChannel()
{
    delete [] buffer_;
}

void AChannel::updateToTime( double time )
{
    unsigned offset = (unsigned) (time * ASE::samplingRate);

    updateTo( offset );
}

void AChannel::updateTo( unsigned offset )
{
    if( bufofs_ < offset ) {
        // Expand buffer if needed
        if( buflen_ < offset ) {
            // Allocate new buffer
            buflen_ = offset;

            AFloat * buf = new AFloat [ buflen_ ];

            // Copy content and dispose old buffer
            if( buffer_ != 0 ) {
                memcpy( buf, buffer_, sizeof(AFloat)*bufofs_ );
                delete [] buffer_;
            }

            buffer_ = buf;
        }

        if( enabled_ ) {
            // Invoke the update implementation
            updateBuffer( buffer_ + bufofs_, offset - bufofs_, offset );
        }
        else {
            // Fill the buffer with zeroes
            while( bufofs_ < offset ) {
                buffer_[ bufofs_ ] = 0;
                bufofs_++;
            }
        }

        // Update the buffer offset
        bufofs_ = offset;
    }
    else if( offset == 0 ) {
        // Reset buffer offset
        updateBuffer( 0, 0, 0 );
        bufofs_ = 0;
    }
}

void AChannel::mixStream( int * dest, AFloat gain )
{
    unsigned len = bufofs_;
    AFloat * buf = buffer_;

    while( len > 0 ) {
        *dest += (int) (gain * (*buf));

        dest++;

        buf++;
        len--;
    }
}

void AChannel::mixStream( int * dest, AFloat gain, int outMin, int outMax )
{
    unsigned len = bufofs_;
    AFloat * buf = buffer_;

    while( len > 0 ) {
        int v = (int) (gain * (*buf));

        if( v > outMax ) v = outMax;
        if( v < outMin ) v = outMin;

        *dest += v;

        dest++;

        buf++;
        len--;
    }
}
