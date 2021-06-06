/*
    Nibbler arcade machine emulator

    Copyright (c) 2011 Alessandro Scotti
*/
#ifndef NIBBLER_H_
#define NIBBLER_H_

#include "vanguard.h"

#include <ase/ase_bandpass_filter.h>
#include <ase/ase_clipper.h>
#include <ase/ase_inverter.h>
#include <ase/ase_latch.h>
#include <ase/ase_noise.h>
#include <ase/ase_capacitor_with_switch.h>
#include <ase/ase_lowpass_filter.h>

struct NibblerBoard : public N6502Environment
{
    NibblerBoard();

    ~NibblerBoard();

    virtual void reset();

    virtual void run();

    // Implementation of the N6502Environment interface
    unsigned char readByte( unsigned addr );
    void writeByte( unsigned, unsigned char );
    
    static void buildWaveform( int * waveform, int mask );

    unsigned char   ram_[60*1024];
    unsigned char   dsw0_;
    unsigned char   port0_;
    unsigned char   port1_;
    unsigned char   port2_;
    unsigned char   coin_slot_;
    unsigned        back_color_;
    unsigned        char_bank_;
    unsigned        frame_counter_; // How many times run() has been called since last reset() 
    int             scroll_x_;
    int             scroll_y_;
    N6502 *         cpu_;
    unsigned char   sound_rom_[0x1800];
    VanguardSoundBoard sound_board_;
    
    // Explosion
    AWhiteNoise * a_noise_;
    ALowPassRCFilter * a_noise_rc_filter_;
    ALatch * a_hit_latch_;
    AClipperLo * a_hit_diode_;
    ACapacitorWithSwitch * a_hit_;
    AActiveBandPassFilter * a_hit_filter_;
    ALowPassRCFilter * a_rc_filter_1_;
    ALowPassRCFilter * a_rc_filter_2_;
    ALowPassRCFilter * a_rc_filter_3_;
};

/**
    Nibbler machine driver.

    @author Alessandro Scotti
    @version 1.0
*/
class Nibbler : public TStandardMachine
{
public:
    virtual ~Nibbler();

    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );

    virtual void run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate );

    virtual void reset();

    static TMachine * createInstance() {
        return new Nibbler( new NibblerBoard );
    }

protected:
    virtual bool initialize( TMachineDriverInfo * info );

    Nibbler( NibblerBoard * );

    void onVideoROMsChanged();
    TBitmapIndexed * renderVideo();
    
protected:
    Nibbler( const Nibbler & );
    Nibbler & operator = ( const Nibbler & );

    NibblerBoard *  main_board_;
    bool            refresh_roms_;      // Set to true if ROM changed since last frame
    unsigned        back_color_;
    unsigned char   palette_prom_[64];
    unsigned        palette_rgb_[64];   // Decoded palette PROM
    TBitBlock *     char_data_[2];  // Character data for 2 banks of 256 8x8 characters
    TBitBlock       fore_char_data; // Foreground character data (dynamically updated)
};

#endif // NIBBLER_H_
