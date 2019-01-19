/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef EMU_BITBLOCK_H_
#define EMU_BITBLOCK_H_

#include <string.h>

#include "emu_blitter.h"

enum TBitOp {
    opSrcTrans  = 0x0004 | 0,
    opDstTrans  = 0x0004 | 1,
    opXlat      = 0x0008,
    opAdd       = 0x0010,
    opFlipX     = 0x0020,
    opFlipY     = 0x0040,
};

class TBitBlock
{
public:
    TBitBlock( int width, int height ) : width_(width), height_(height) {
        data_ = new unsigned char [width*height];
        clearClipRegion();
    }

    /** Destructor. */
    virtual ~TBitBlock() {
        delete data_;
    }

    int width() const {
        return width_;
    }

    int height() const {
        return height_;
    }

    unsigned char pixel( int x, int y ) const;

    void setPixel( int x, int y, unsigned char value );

    void fill( unsigned char color ) {
        memset( data_, color, width_*height_ );
    }

    void fill( int x, int y, int width, int height, unsigned char color );

    void clear() {
        fill( 0 );
    }

    TBitBlitter * createBlitter( unsigned op, unsigned char color, unsigned char trans, unsigned char * xlat );

    void copy( int x, int y, TBitBlock & block, int sx, int sy, int sw, int sh, unsigned op, TBitBlitter * blitter );

    void copy( int x, int y, TBitBlock & block, unsigned op, TBitBlitter * blitter ) {
        copy( x, y, block, 0, 0, block.width_, block.height_, op, blitter );
    }

    void copy( int x, int y, TBitBlock & block, unsigned op, unsigned char color, unsigned char trans, unsigned char * xlat ) {
        TBitBlitter * blitter = createBlitter( op, color, trans, xlat );
        copy( x, y, block, op, blitter );
        delete blitter;
    }

    unsigned char * data() const {
        return data_;
    }

    unsigned char * scanline_data( int y ) const {
        return data_ + y*width_;
    }

    void clearClipRegion() {
        setClipRegion( 0, 0, width_, height_ );
    }

    void setClipRegion( int x1, int y1, int x2, int y2 ) {
        clip_x1_ = x1;
        clip_y1_ = y1;
        clip_x2_ = x2;
        clip_y2_ = y2;
    }

private:
    int width_;
    int height_;
    int clip_x1_;
    int clip_y1_;
    int clip_x2_;
    int clip_y2_;
    unsigned char * data_;
};

#endif // EMU_BITBLOCK_H_
