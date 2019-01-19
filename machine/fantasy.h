/*
    Fantasy arcade machine emulator

    Copyright (c) 2011 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef FANTASY_H_
#define FANTASY_H_

#include "nibbler.h"

struct FantasyBoard : public NibblerBoard
{
    void writeByte( unsigned, unsigned char ); // To add the speech port

    Hd38880_SimWithSamples hd38880_;
    TSamplePlayer speech_samples_[12];
};

/**
    Fantasy machine driver.

    @author Alessandro Scotti
    @version 1.0
*/
class Fantasy : public Nibbler
{
public:
    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );
    
    virtual void run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate );
    
    static TMachine * createInstance() {
        return new Fantasy( new FantasyBoard );
    }

protected:
    virtual bool initialize( TMachineDriverInfo * info );

    Fantasy( FantasyBoard * );

    unsigned char speech_rom_[0x1800];
};

#endif // FANTASY_H_
