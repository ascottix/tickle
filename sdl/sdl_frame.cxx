/*
    Tickle front-end
    User interface for SDL - Frame management

    Copyright (c) 2014 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "sdl_frame.h"

SDLFrame::SDLFrame( SDL_Renderer * rend, unsigned sampleCount ) {
    rend_ = rend;
    user_data_ = 0;
    sample_count_ = sampleCount;
    video_ = 0;
}

SDLFrame::~SDLFrame() {
    SDL_DestroyTexture( video_ );
}

static SDL_Surface * assignTBitmapToSDLSurface( SDL_Surface * surface, TBitmap * bitmap, bool flip )
{
    // Do nothing if there is not input (it happens in games that update the video every other frame)
    if( bitmap == 0 ) {
        return surface;
    }
    
    int w = bitmap->width();
    int h = bitmap->height();
    
    Uint32 rmask, gmask, bmask, amask;
    
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    if( surface == 0 ) {
        surface = SDL_CreateRGBSurface( 0, w, h, 32, rmask, gmask, bmask, amask );
        if( surface == 0 ) {
            printf("Cannot create surface: %s\n", SDL_GetError());
            return 0;
        }
    }
    
    if( bitmap->format() == bfIndexed ) {
        TBitmapIndexed * bm = reinterpret_cast<TBitmapIndexed *>( bitmap );
        
        const unsigned char * bits = bm->bits()->data();
        const unsigned * palette = bm->palette()->data();

        // Set pixels
        Uint32 dest_palette[256];
        
        for( int i=0; i<256; i++ ) {
            unsigned char r, g, b;
            TPalette::decodeColor( palette[i], &r, &g, &b );
            dest_palette[i] = SDL_MapRGB( surface->format, r, g, b );
        }
        
        Uint32 * dest = (Uint32 *) surface->pixels;
        
        int n = w*h;
        
        if( flip ) {
            dest += n;
            for( ; n>0; n-- ) {
                *--dest = dest_palette[ *bits++ ];
            }
        }
        else {
            for( ; n>0; n-- ) {
                *dest++ = dest_palette[ *bits++ ];
            }
        }
    }
    else if( bitmap->format() == bfRGB ) {
        TBitmapRGB * bm = reinterpret_cast<TBitmapRGB *>( bitmap );
        
        const unsigned * src = bm->data();
        Uint32 * dst = (Uint32 *) surface->pixels;
        
        for( int n=w*h; n>0; n-- ) {
            unsigned c = *src++;
            
            *dst++ = SDL_MapRGB( surface->format, c >> 16, c >> 8, c );
        }
    }
    
    return surface;
}

void SDLFrame::setVideo( TBitmap * video, bool flipped ) {
    SDL_Surface * buffer = assignTBitmapToSDLSurface( 0, video, flipped );
    
    if( buffer ) {
        video_ = SDL_CreateTextureFromSurface( rend_, buffer );
        SDL_FreeSurface(buffer );
    }
}
