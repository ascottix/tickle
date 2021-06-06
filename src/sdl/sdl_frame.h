/*
    Tickle front-end
    User interface for SDL

    Copyright (c) 2014 Alessandro Scotti
*/
#ifndef SDL_FRAME_H_
#define SDL_FRAME_H_

#include <SDL2/SDL.h>

#include <emu/emu_frame.h>
#include <emu/emu_math.h>
#include <emu/emu_mixer.h>

class SDLFrame : public TFrame
{
public:
    SDLFrame( SDL_Renderer * rend, unsigned sampleCount );

    virtual ~SDLFrame();

    virtual void setVideo( TBitmap * video, bool flipped );

    virtual TMixer * getMixer() {
        return &mixer_;
    }
    
    SDL_Texture * getTexture() {
        return video_;
    }
    
    SDL_Texture * getAndDetachTexture() {
        SDL_Texture * txt = video_;
        video_ = 0;
        return txt;
    }
    
    unsigned getSampleCount() const {
        return sample_count_;
    }
    
    unsigned getUserData() const {
        return user_data_;
    }
    
    void setUserData( unsigned value ) {
        user_data_ = value;
    }

private:
    SDL_Renderer * rend_;
    SDL_Texture * video_;
    TMixerMono mixer_;
    unsigned sample_count_;
    unsigned user_data_;
};

#endif // WIN_FRAME_H_
