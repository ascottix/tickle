/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#ifndef EMU_PNG_H_
#define EMU_PNG_H_

#include "emu_bitmap.h"
#include "emu_iostream.h"

/**
    Reads a PNG file from the specified input stream.

    @return null on error, or a TBitmap object that may be an
    instance of either TBitmapIndexed or TBitmapRGB.
*/
TBitmap * createBitmapFromPNG( TInputStream * );

#endif // EMU_PNG_H_
