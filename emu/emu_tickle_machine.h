/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef EMU_TICKLE_MACHINE_H_
#define EMU_TICKLE_MACHINE_H_

#include "emu_driver.h"

class TTickleMachine : public TMachine
{
public:
    static TMachine * create() {
        return TMachine::createInstance( TTickleMachine::createInstance );
    }

    /** Destructor. */
    ~TTickleMachine();
    
    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );

    virtual bool handleInputEvent( unsigned device, unsigned param, void * data );

    virtual void run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate );

    virtual void reset() {
    }

protected:
    TTickleMachine();

    virtual bool initialize( TMachineDriverInfo * info );

    static TMachine * createInstance() {
        return new TTickleMachine();
    }

private:
    void drawChar( int x, int y, char c );
    void drawString( int x, int y, const char * s );

    enum {
        stTestMode = 0x01,
        stP1StartPressed = 0x02,
        stCoin1Pressed = 0x04,
        stP1Action1Pressed = 0x08
    };

    TBitmapRGB * screen_;
    TBitmapRGB * splash_;
    TBitmapRGB * font_;
    TOptionToPortHandler option_handler_;
    unsigned last_device_;
    unsigned last_device_param_;
    unsigned status_;
    unsigned char dip_switches_;
};

#endif // EMU_TICKLE_MACHINE_H_
