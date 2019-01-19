/*
    Tickle class library

    Copyright (c) 2011 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef EMU_CHAR_DECODER_H_
#define EMU_CHAR_DECODER_H_

// Full info (up to 16x16 and 4 bit planes)
struct TDecodeCharInfo
{
    int width;          // Width in pixels
    int height;         // Height in pixels
    int nplanes;        // Number of bit planes
    int planeofs[4];    // Offset of bit plane (in bits)
    int xofs[16];       // Offset of pixel x coordinate in bits relative to one bit plane (in bits)
    int yofs[16];       // Offset of pixel y coordinate in bits relative to one bit plane (in bits)
};

// Slightly simpler and smaller version for 8x8 characters and tiles
struct TDecodeCharInfo8x8
{
    int nplanes;        // Number of bit planes
    int planeofs[4];    // Offset of bit plane (in bits)
    int xofs[8];        // Offset of pixel x coordinate in bits relative to one bit plane (in bits)
    int yofs[8];        // Offset of pixel y coordinate in bits relative to one bit plane (in bits)
};

void decodeChar( unsigned char * buf, const TDecodeCharInfo & info, const unsigned char * source, int offset_bits );

void decodeCharSet( unsigned char * buf, const TDecodeCharInfo & info, const unsigned char * source, int total, int delta_bits );

void decodeChar8x8( unsigned char * buf, const TDecodeCharInfo8x8 & info, const unsigned char * source );

void decodeCharSet8x8( unsigned char * buf, const TDecodeCharInfo8x8 & info, const unsigned char * source, int total, int delta );

#endif // EMU_CHAR_DECODER_H_
