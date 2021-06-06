/*
 Tickle class library
 
 Copyright (c) 2014 Alessandro Scotti

 */
#ifndef __tickle__sdl_main__
#define __tickle__sdl_main__

#include <SDL2/SDL.h>

#include "sdl_frame.h"
#include "fifo.h"

enum {
    SDLTickleEvent_AddFrame = 0,
    SDLTickleEvent_RenderTexture,
    SDLTickleEvent_DestroyTexture,
    // Leave this line for last, it contains how many user events have been defined
    SDLTickleEvent_Count
};

class TMachine;

struct SDLMainOptions {
    int w; // Window width
    int h; // Window height
    bool fullscreen; // Fullscreen on/off
    int audiofreq; // Audio frequency (sampling rate)
    
    SDLMainOptions() {
        w = 224;
        h = 288;
        fullscreen = false;
        audiofreq = 44100;
    }
    
    SDLMainOptions & operator = ( const SDLMainOptions & o ) {
        w = o.w;
        h = o.h;
        fullscreen = o.fullscreen;
        audiofreq = o.audiofreq;
        return *this;
    }
};

class SDLMain {
public:
    SDLMain();
    
    virtual ~SDLMain() {
        term();
    }
    
    bool init( const SDLMainOptions & options );
    
    void term();
    
    bool go( TMachine * machine );
    
    void render( SDL_Texture * texture ) const;
    
    void sleep( unsigned ms ) const {
        SDL_Delay( (Uint32) ms );
    }
    
    void audio_lock() const {
        SDL_LockAudioDevice( adid_ );
    }
    
    void audio_unlock() const {
        SDL_UnlockAudioDevice( adid_ );
    }
    
    void audio_stop() const {
        SDL_PauseAudioDevice( adid_, 1 );
    }
    
    void audio_play() const {
        SDL_PauseAudioDevice( adid_, 0 );
    }

    void add_frame( TMachine * machine );
    
    void add_frame( SDLFrame * frame );
    
    void push_user_event( Sint32 code, void * data1 = 0, void * data2 = 0 );
    
    bool joystick_status( int joystick, Sint16 * x, Sint16 * y, unsigned * buttons );
    
    void set_audio_rate( int rate );
    
    SDL_Window * window() {
        return window_;
    }
    
    SDL_Renderer * renderer() {
        return rend_;
    }
    
private:
    static void audioCallbackStub( void * userdata, Uint8 * stream, int len );
    
    static Uint32 videoCallbackStub( Uint32 interval, void * param );
    
    bool error( const char * info );
    
    void reset();
    
    void audioStreamCallback( Uint8 * stream, int len );
    
    unsigned videoStreamCallback( unsigned interval );
    
    SDL_Window * window_;
    Uint32 window_flags_;
    SDL_Renderer * rend_;
    SDL_AudioDeviceID adid_;
    SDL_Joystick * joystick_[2];
    Uint32 user_event_type_;
    SDLMainOptions options_;
    SDL_TimerID video_tid_;
    unsigned frame_delay_;
    SDLFrame * cur_frame_;
    Fifo audio_q_;
    Fifo video_q_;
};

#endif /* defined(__tickle__sdl_main__) */
