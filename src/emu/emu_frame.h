/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#ifndef EMU_FRAME_H_
#define EMU_FRAME_H_

#include "emu_bitmap.h"
#include "emu_mixer.h"

enum TForceFeedbackEffect {
    fxFF_Explosion,
    fxFF_Bump
};

class TFrame
{
    unsigned flags_; // Flags are mainly used for debugging
    unsigned stamp_;
    void * userData_;
    
public:
    TFrame() {
        flags_ = 0;
        stamp_ = 0;
        userData_ = 0;
    }

    /** Destructor. */
    virtual ~TFrame() {
    }
    
    void setFlags( unsigned flags ) {
        flags_ = flags;
    }
    
    unsigned getFlags() const {
        return flags_;
    }
    
    void setStamp( unsigned stamp ) {
        stamp_ = stamp;
    }
    
    unsigned getStamp() {
        return stamp_;
    }
    
    void setUserData( void * data ) {
        userData_ = data;
    }
    
    void * getUserData() const {
        return userData_;
    }

    virtual void setVideo( TBitmap * screen, bool flipped = false ) = 0;

    virtual TMixer * getMixer() = 0;

    virtual void playForceFeedbackEffect( int player, TForceFeedbackEffect fx, unsigned param );

    virtual void stopForceFeedbackEffect( int player );

    virtual void setPlayerLight( int player, bool on );

    virtual void incrementCoinCounter( int counter );
};

#endif // EMU_FRAME_H_
