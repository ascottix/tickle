/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#include "emu_bitmap.h"

TBitmap::TBitmap( int width, int height, TBitmapFormat format ) : 
    width_(width), height_(height), format_(format)
{
}

TBitmapIndexed::TBitmapIndexed( int width, int height, TPalette * palette ) : 
    TBitmap( width, height, bfIndexed ), palette_(palette) 
{
    bits_ = new TBitBlock( width, height );
    clear();
}

void TBitmapIndexed::clear()
{
    bits_->clear();
}

bool TBitmapIndexed::draw( int x, int y, TBitmap & bitmap )
{
    bool result = false;

    // Not yet implemented

    return result;
}

TBitmapRGB::TBitmapRGB( int width, int height ) :
    TBitmap( width, height, bfRGB )
{
    bits_ = new unsigned [width * height];
}
 
unsigned TBitmapRGB::pixel( int x, int y )
{
    unsigned result = 0;

    if( (x >= 0) && (x < width()) && (y >= 0) && (y < height()) ) {
        result = bits_[ y*width() + x ];
    }

    return result;
}

void TBitmapRGB::setPixel( int x, int y, unsigned color )
{
    if( (x >= 0) && (x < width()) && (y >= 0) && (y < height()) ) {
        bits_[ y*width() + x ] = color;
    }
}

void TBitmapRGB::clear()
{
    memset( bits_, 0, sizeof(unsigned)*width()*height() );
}

bool TBitmapRGB::draw( int x, int y, TBitmap & bitmap )
{
    bool result = false;

    // Not yet implemented

    return result;
}
