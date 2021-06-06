/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#ifndef EMU_BITMAP_H_
#define EMU_BITMAP_H_

#include <string.h>

#include "emu_bitblock.h"
#include "emu_palette.h"

enum TBitmapFormat 
{
    bfIndexed,  // 8 bits per pixel (some of which may be unused) with external palette
    bfRGB,      // 24 bits per pixel in RGB format
    bfCustom
};

class TBitmap
{
public:
    /** Destructor. */
    virtual ~TBitmap() {
    }

    int width() const {
        return width_;
    }

    int height() const {
        return height_;
    }

    TBitmapFormat format() const {
        return format_;
    }

    virtual unsigned pixel( int x, int y ) = 0;

    virtual void setPixel( int x, int y, unsigned color ) = 0;

    virtual void clear() = 0;

    virtual bool draw( int x, int y, TBitmap & bitmap ) = 0;

protected:
    TBitmap( int width, int height, TBitmapFormat format );

private:
    int width_;
    int height_;
    TBitmapFormat format_;
};

class TBitmapIndexed : public TBitmap
{
public:
    TBitmapIndexed( int width, int height, TPalette * palette );
 
    /** Destructor. */
    virtual ~TBitmapIndexed() {
        delete bits_;
    }

    virtual unsigned pixel( int x, int y ) {
        return palette_->color( bits_->pixel( x, y ) );
    }

    virtual void setPixel( int x, int y, unsigned color ) {
        bits_->setPixel( x, y, palette_->getNearestColor(color) );
    }

    virtual void clear();

    virtual bool draw( int x, int y, TBitmap & bitmap );

    TBitBlock * bits() const {
        return bits_;
    }

    TPalette * palette() const {
        return palette_;
    }

private:
    TBitBlock * bits_;
    TPalette * palette_;
};

class TBitmapRGB : public TBitmap
{
public:
    TBitmapRGB( int width, int height );
 
    /** Destructor. */
    virtual ~TBitmapRGB() {
        delete [] bits_;
    }

    virtual unsigned pixel( int x, int y );

    virtual void setPixel( int x, int y, unsigned color );

    virtual void clear();

    virtual bool draw( int x, int y, TBitmap & bitmap );

    unsigned * data() const {
        return bits_;
    }

    unsigned * scanline_data( int y ) const {
        return bits_ + y*width();
    }

private:
    unsigned * bits_;
};

#endif // EMU_BITMAP_H_
