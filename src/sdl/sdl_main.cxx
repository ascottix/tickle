/*
    Tickle class library
    SDL driver
 
    Copyright (c) 2014-2021 Alessandro Scotti
*/
#include "sdl_main.h"

#include <emu/emu.h>

const int SamplesPerCallback = 1024;

SDLMain::SDLMain() {
    reset();
}

void SDLMain::reset() {
    window_ = 0;
    window_flags_ = 0;
    rend_ = 0;
    adid_ = 0;
    frame_delay_ = 0;
    video_tid_ = 0;
    int maxj = sizeof(joystick_) / sizeof(joystick_[0]);
    for( int i=0; i<maxj; i++ ) {
        joystick_[i] = 0;
    }
}

bool SDLMain::error( const char * info ) {
    printf( "%s: %s\n", info, SDL_GetError() );
    
    return false;
}

bool SDLMain::init( const SDLMainOptions & options ) {
    options_ = options;
    
    bool ok = true;

    if( ok ) {
        if( SDL_Init(SDL_INIT_EVERYTHING) != 0 ) {
            ok = error( "Cannot initialize SDL" );
        }
    }
    
    if( ok ) {
        int maxj = sizeof(joystick_) / sizeof(joystick_[0]);
        int numj = SDL_NumJoysticks();
        
        numj = TMath::min( numj, maxj );
        
        for( int i=0; i<numj; i++ ) {
            joystick_[i] = SDL_JoystickOpen(i);
        }
    }
    
    if( ok ) {
        SDL_AudioSpec want;
        SDL_AudioSpec have;
        
        SDL_zero(want);
        
        want.freq = options_.audiofreq;
        want.format = AUDIO_S16SYS;
        want.channels = 1; // Mono
        want.samples = SamplesPerCallback;
        want.callback = audioCallbackStub;
        want.userdata = this;
        
        adid_ = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0 );
        
        if( adid_ == 0 ) {
            ok = error( "Cannot open audio device" );
        }
    }
    
    if( ok ) {
        window_flags_ = SDL_WINDOW_SHOWN;
        
        if( options_.fullscreen ) {
            window_flags_ |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        
        int ww = options_.w;
        int wh = options_.h;
        
        window_ = SDL_CreateWindow( "Tickle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ww, wh, window_flags_ );
        
        if( ! window_ ) {
            ok = error( "Cannot create main window" );
        }
        else {
            // If we haven't got the requested size, fake the fullscreen mode (which will adapt to the window)
            int aw, ah;
            
            SDL_GetWindowSize( window_, &aw, &ah );

            if( aw != ww || ah != wh ) window_flags_ |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
    }
    
    if( ok ) {
        rend_ = SDL_CreateRenderer( window_, -1, 0 );
        
        if( ! rend_ ) {
            ok = error( "Cannot create renderer" );
        }
        else {
            SDL_RenderClear( rend_ );
        }
    }
    
    if( ok ) {
        user_event_type_ = SDL_RegisterEvents( SDLTickleEvent_Count );
        
        if( user_event_type_ == (Uint32)-1 ) {
            ok = error( "Cannot register user events" );
        }
    }
    
    return ok;
}

void SDLMain::term() {
    SDL_CloseAudioDevice( adid_ );
    
    // TODO: empty queues

    int maxj = sizeof(joystick_) / sizeof(joystick_[0]);
    for( int i=0; i<maxj; i++ ) {
        if( joystick_[i] && SDL_JoystickGetAttached(joystick_[i]) ) {
            SDL_JoystickClose(joystick_[i]);
        }
    }

    SDL_DestroyRenderer( rend_ );
    SDL_DestroyWindow( window_ );
    SDL_Quit();
    
    reset();
}

void SDLMain::render( SDL_Texture * texture ) const {
    if( texture != 0 ) {
        if( window_flags_ & (SDL_WINDOW_FULLSCREEN|SDL_WINDOW_FULLSCREEN_DESKTOP) ) {
            int aw, ah;
            
            SDL_GetWindowSize( window_, &aw, &ah );
            
            // Fix height, adjust width
            int wh = ah;
            int ww = (wh * options_.w) / options_.h;
            
            // If that doesn't work, fix width and ajdust height
            if( ww > aw ) {
                ww = aw;
                wh = (ww * options_.h) / options_.w;
            }
            
            SDL_Rect rect;
            
            rect.x = (aw - ww) / 2;
            rect.y = (ah - wh) / 2;
            rect.w = ww;
            rect.h = wh;
            
            SDL_RenderCopy( rend_, texture, NULL, &rect );
        }
        else {
            SDL_RenderCopy( rend_, texture, 0, 0 );
        }
        
        SDL_RenderPresent( rend_ );
        SDL_DestroyTexture( texture );
    }
}

void SDLMain::add_frame( TMachine * machine ) {
    int audioSamplesPerFrame = options_.audiofreq / machine->getDriverInfo()->machineInfo()->framesPerSecond;
    SDLFrame * frame = new SDLFrame( rend_, audioSamplesPerFrame );
    machine->run( frame, audioSamplesPerFrame, options_.audiofreq );
    add_frame( frame );
}

void SDLMain::add_frame( SDLFrame * frame ) {
    audio_lock();
    audio_q_.append( frame );
    audio_unlock();
}

void SDLMain::push_user_event( Sint32 code, void * data1, void * data2 ) {
    SDL_Event event;
    SDL_zero( event );
    event.type = user_event_type_;
    event.user.code = code;
    event.user.data1 = data1;
    event.user.data2 = data2;
    SDL_PushEvent( &event );
}

bool SDLMain::joystick_status( int joystick, Sint16 * x, Sint16 * y, unsigned * buttons ) {
    bool ok = false;
    
    SDL_Joystick * j = joystick_[joystick];

    if( j ) {
        *x = SDL_JoystickGetAxis( j, 0 );
        *y = SDL_JoystickGetAxis( j, 1 );
        *buttons = 0;
        for( int i=0; i<SDL_JoystickNumButtons(j); i++ ) {
            if( SDL_JoystickGetButton( j, i ) ) {
                *buttons |= 1 << i;
            }
        }
        
        ok = true;

        // TODO: properly handle calibration
        if( *x >= -1024 && *x < 1024 ) *x = 0;
        if( *y >= -1024 && *y < 1024 ) *y = 0;
    }
    
    return ok;
}

bool SDLMain::go( TMachine * machine ) {
    int audioSamplesPerFrame = options_.audiofreq / machine->getDriverInfo()->machineInfo()->framesPerSecond;
    
    // Initialize the frame queue with enough frames
    for( int n=0; n<audioSamplesPerFrame; n+=SamplesPerCallback ) {
        add_frame( machine );
    }
    add_frame( machine );
    
    frame_delay_ = 1000 / machine->getDriverInfo()->machineInfo()->framesPerSecond;
    
    cur_frame_ = new SDLFrame( rend_, 0 );
    
    audio_play();
    
    return true;
}

Uint32 SDLMain::videoCallbackStub( Uint32 interval, void * param ) {
    SDLMain * sdl = (SDLMain *) param;
    
    return (Uint32) sdl->videoStreamCallback( (unsigned)interval );
}

unsigned SDLMain::videoStreamCallback( unsigned interval ) {
    if( ! video_q_.empty() ) {
        // Show last queued element
        SDL_Texture * texture = 0;
        
        do {
            audio_lock();
            SDL_Texture * t = (SDL_Texture *) video_q_.remove();
            audio_unlock();
            
            if( t != 0 ) { // Texture may be null if the machine driver has skipped a video frame (e.g. Pacman)
                push_user_event( SDLTickleEvent_DestroyTexture, texture );
                texture = t;
            }
        } while( ! video_q_.empty() );

        // Can't render inside a timer callback, send message to main loop
        push_user_event( SDLTickleEvent_RenderTexture, texture );
    }
    
    return frame_delay_;
}

void SDLMain::audioCallbackStub( void * userdata, Uint8 * stream, int len ) {
    SDLMain * sdl = (SDLMain *) userdata;
    
    sdl->audioStreamCallback( stream, len );
}

void SDLMain::audioStreamCallback( Uint8 * stream, int len ) {
    if( cur_frame_ == 0 ) {
        // Shouldn't happen... we're probably recovering from an underrun
        cur_frame_ = (SDLFrame *) audio_q_.remove();
        if( cur_frame_ == 0 ) {
            memset( stream, 0, len );
            return;
        }
    }
    
    int16_t * stream16 = (int16_t *) stream;
    
    len /= 2; // Convert bytes in samples
    
    while( len > 0 ) {
        unsigned cf_ofs = cur_frame_->getUserData();
        unsigned cf_len = cur_frame_->getSampleCount();
        unsigned cf_avail = cf_len - cf_ofs;

        if( cf_avail > (unsigned)len ) cf_avail = len;

        cur_frame_->setUserData( cf_ofs + cf_avail );
        
        // Copy into destination stream
        int voices = cur_frame_->getMixer()->maxVoicesPerChannel();
        const int * buf = cur_frame_->getMixer()->buffer(0);
        
        if( buf != 0 ) {
            buf += cf_ofs;
            
            for( unsigned i=0; i<cf_avail; i++ ) {
                *stream16++ = *buf++ * (128 / voices);
            }
        }
        else {
            for( unsigned i=0; i<cf_avail; i++ ) {
                *stream16++ = 0;
            }
        }
        
        // Update frame
        len -= cf_avail;
        
        // If current frame is empty, dispose it and get a new one from the queue
        if( cf_avail == 0 ) {
            delete cur_frame_;
            
            cur_frame_ = (SDLFrame *) audio_q_.remove();
            
            if( cur_frame_ == 0 ) {
                // Ouch, we run out of frames... request an extra frame, clear the rest of the buffer and return
                push_user_event( SDLTickleEvent_AddFrame );
                memset( stream16, 0, len*2 );
                return;
            }
            
            SDL_Texture * t = cur_frame_->getAndDetachTexture();
            
            video_q_.append( t );
            
            // Hmmm: shall this be moved to the event loop?
            if( video_tid_ == 0 ) {
                video_tid_ = SDL_AddTimer( 1, videoCallbackStub, this );
            }
            
            push_user_event( SDLTickleEvent_AddFrame );
        }
    }
}
