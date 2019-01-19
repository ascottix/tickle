/*
    Namco 05xx (starfield generator) custom chip emulator

    Copyright (c) 2011 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef NAMCO_05XX_
#define NAMCO_05XX_

#include <emu/emu_bitmap.h>

struct Namco05xx {
    unsigned char state_;
    int scroll_x_;
    int scroll_y_;
    
    Namco05xx();
    
    void reset();
    
    void writeRegister( unsigned index, unsigned char value );
    
    void update();
    
    void render( TBitmapIndexed * screen );
};

#endif // NAMCO_05XX_
