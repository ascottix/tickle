/*
    Vanguard arcade machine emulator

    Copyright (c) 2011 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef VANGUARD_H_
#define VANGUARD_H_

#include <emu/emu_standard_machine.h>
#include <cpu/n6502.h>
#include <sound/sn76477.h>

#include "vanguard_soundboard.h"

struct VanguardBoard : public N6502Environment
{
    VanguardBoard();

    ~VanguardBoard();

    virtual void reset();

    virtual void run();

    // Implementation of the N6502Environment interface
    unsigned char readByte( unsigned addr );
    void writeByte( unsigned, unsigned char );
    
    void writeToSpeechPort( unsigned char b );
    
    unsigned char   ram_[48*1024]; // RAM and ROM
    unsigned char   dsw0_;
    unsigned char   port0_;
    unsigned char   port1_;
    unsigned char   port2_;
    unsigned char   coin_slot_;
    unsigned char   cheat_options_;
    unsigned char   cheat_multi_fire_;
    unsigned char   o_port_3100_;
    unsigned        back_color_;
    unsigned        char_bank_;
    unsigned        frame_counter_; // How many times run() has been called since last reset() 
    int             scroll_x_;
    int             scroll_y_;
    SN76477         sn_bomb_;
    SN76477         sn_shot_b_;
    N6502 *         cpu_;
    unsigned char   speech_rom_[0x1800];
    unsigned char   sound_rom_[0x1000];
    VanguardSoundBoard sound_board_;
    Hd38880_SimWithSamples hd38880_;
    TSamplePlayer   speech_samples_[16];
    TSamplePlayer   sample_bomb_;
    TSamplePlayer   sample_shot_a_;
};

/**
    Vanguard machine driver.

    @author Alessandro Scotti
    @version 1.0
*/
class Vanguard : public TStandardMachine
{
public:
    virtual ~Vanguard();

    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );

    virtual void run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate );

    virtual void reset();

    static TMachine * createInstance() {
        return new Vanguard( new VanguardBoard );
    }

    static TBitmapIndexed * renderVideo( TBitmapIndexed * screen, unsigned char * ram, TBitBlock & back_char_data, TBitBlock & fore_char_data, unsigned back_color, int scroll_x, int scroll_y );
    
    static void decodeCharSet( unsigned char * b_src, unsigned char * b_dst, unsigned plane_offset );
    
protected:
    virtual bool initialize( TMachineDriverInfo * info );

    Vanguard( VanguardBoard * );

    void onVideoROMsChanged();
    TBitmapIndexed * renderVideo();
    
private:
    Vanguard( const Vanguard & );
    Vanguard & operator = ( const Vanguard & );

    VanguardBoard * main_board_;
    bool            refresh_roms_;      // Set to true if ROM changed since last frame
    unsigned        back_color_;
    unsigned char   palette_prom_[64];
    unsigned        palette_rgb_[64];   // Decoded palette PROM
    TBitBlock       back_char_data_;
    TBitBlock       fore_char_data_;
};

#endif // VANGUARD_H_
