/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#include <assert.h>

#include "emu_bitblock.h"

void TBitBlock::fill( int x, int y, int width, int height, unsigned char color )
{
    int x2 = x + width;
    int y2 = y + height;

    // Exit if specified rectangle is out of range
    if( (x >= width_) || (y >= height_) || (x2 < 0) || (y2 < 0) || (width < 0) || (height < 0) ) {
        return;
    }

    // Clip variables
    if( x < 0 ) x = 0;
    if( y < 0 ) y = 0;
    if( x2 > width_ ) x2 = width_;
    if( y2 > height_ ) y2 = height_;

    unsigned char * data = data_ + x + y*width_;

    while( y < y2 ) {
        memset( data, color, x2-x );
        data += width_;
        y++;
    }
}

unsigned char TBitBlock::pixel( int x, int y ) const 
{
    unsigned char result = 0;

    if( (x >= 0) && (x < width_) && (y >= 0) && (y < height_) ) {
        result = data_[x+y*width_];
    }

    return result;
}

void TBitBlock::setPixel( int x, int y, unsigned char value ) 
{
    if( (x >= 0) && (x < width_) && (y >= 0) && (y < height_) ) {
        data_[x+y*width_] = value;
    }
}

void TBitBlock::copy( int x, int y, TBitBlock & block, int sx, int sy, int sw, int sh, unsigned op, TBitBlitter * blitter )
{
    // Clip source rectangle
    if( (blitter == 0) || (sw <= 0) || (sh <= 0) || (sx >= block.width_) || (sy >= block.height_) ) {
        return;
    }

    if( sx < 0 ) {
        sw += sx; // Note: we add a negative value!
        sx = 0;
    }

    if( sy < 0 ) {
        sh += sy; // Note: we add a negative value!
        sy = 0;
    }

    if( sx+sw > block.width_ ) {
        sw = block.width_ - sx;
    }

    if( sy+sh > block.height_ ) {
        sh = block.height_ - sy;
    }

    // Get source pointer
    unsigned char * src = block.data_ + sy * block.width_ + sx;
    int src_step = block.width_;

    if( op & opFlipX ) {
        src += sw;
    }

    if( op & opFlipY ) {
        src += (sh-1)*block.width_;
        src_step = -src_step;
    }

    // Clip destination rectangle
    int x2 = x + sw;
    int y2 = y + sh;

    if( x < clip_x1_ ) {
        int d = x - clip_x1_;
        src += op & opFlipX ? +d : -d;
        x = clip_x1_;
    }

    if( x2 > clip_x2_ ) {
        x2 = clip_x2_;
    }

    if( y < clip_y1_ ) {
        int d = clip_y1_ - y;
        src += op & opFlipY ? -d*block.width_ : +d*block.width_;
        y = clip_y1_;
    }

    if( y2 > clip_y2_ ) {
        y2 = clip_y2_;
    }

    // Do the copy
    int len = x2 - x;

    if( len > 0 ) {
        unsigned char * dst = data_ + x + y*width_;

        while( y < y2 ) {
            blitter->blit( dst, src, len );
            src += src_step;
            dst += width_;
            y++;
        }
    }
}

TBitBlitter * TBitBlock::createBlitter( unsigned op, unsigned char color, unsigned char trans, unsigned char * xlat )
{
    op &= ~opFlipY;

    switch( op ) {
    case 0:                                 return new TBltCopy;
    case opFlipX:                           return new TBltCopyReverse;
    case opAdd:                             return new TBltAdd( color );
    case opAdd|opFlipX:                     return new TBltAddReverse( color );
    case opAdd|opSrcTrans:                  if( trans == 0 ) return new TBltAddSrcZeroTrans( color ); else return new TBltAddSrcTrans( color, trans );
    case opAdd|opSrcTrans|opFlipX:          if( trans == 0 ) return new TBltAddSrcZeroTransReverse( color ); else return new TBltAddSrcTransReverse( color, trans );
    case opAdd|opXlat:                      return new TBltAddXlat( color, xlat );
    case opAdd|opXlat|opFlipX:              return new TBltAddXlatReverse( color, xlat );
    case opAdd|opXlat|opSrcTrans:           return new TBltAddXlatSrcTrans( color, trans, xlat );
    case opAdd|opXlat|opSrcTrans|opFlipX:   return new TBltAddXlatSrcTransReverse( color, trans, xlat );
    case opAdd|opXlat|opDstTrans:           return new TBltAddXlatDstTrans( color, trans, xlat );
    case opAdd|opXlat|opDstTrans|opFlipX:   return new TBltAddXlatDstTransReverse( color, trans, xlat );
    }

    assert( false );

    return 0;
}

