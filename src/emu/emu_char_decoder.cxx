/*
    Tickle class library

    Copyright (c) 2011 Alessandro Scotti
*/
#include "emu_char_decoder.h"

static const unsigned char BitMask[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

void decodeChar8x8( unsigned char * buf, const TDecodeCharInfo8x8 & info, const unsigned char * source )
{
    for( int y=0; y<8; y++ ) {
        for( int x=0; x<8; x++ ) {
            unsigned baseofs = info.yofs[y] + info.xofs[x];
            
            unsigned char b = 0;
            
            for( int plane=0; plane<info.nplanes; plane++ ) {
                unsigned bitofs = baseofs + info.planeofs[plane];
                
                if( source[bitofs / 8] & BitMask[bitofs % 8] ) {
                    b |= BitMask[plane];
                }
            }
            
            *buf++ = b;
        }
    }
}

void decodeCharSet8x8( unsigned char * buf, const TDecodeCharInfo8x8 & info, const unsigned char * source, int total, int delta )
{
    for( int i=0; i<total; i++ ) {
        decodeChar8x8( buf, info, source );
        buf += 8*8;
        source += delta; // Here delta is in bytes (use the "full" version if this is not the case)
    }
}

void decodeChar( unsigned char * buf, const TDecodeCharInfo & info, const unsigned char * source, int offset_bits )
{
    for( int y=info.height-1; y>=0; y-- ) {
        for( int x=info.width-1; x>=0; x-- ) {
            unsigned baseofs = info.yofs[y] + info.xofs[x] + offset_bits;
            
            unsigned char b = 0;
            
            for( int plane=0; plane<info.nplanes; plane++ ) {
                unsigned bitofs = baseofs + info.planeofs[plane];
                
                if( source[bitofs / 8] & BitMask[bitofs % 8] ) {
                    b |= BitMask[plane];
                }
            }
            
            *buf++ = b;
        }
    }
}

void decodeCharSet( unsigned char * buf, const TDecodeCharInfo & info, const unsigned char * source, int total, int delta_bits )
{
    int offset_bits = 0;
    int buf_delta = info.width * info.height;
    
    for( int i=0; i<total; i++ ) {
        decodeChar( buf, info, source, offset_bits );
        buf += buf_delta;
        offset_bits += delta_bits;
    }
}
