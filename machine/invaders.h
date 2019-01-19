/*
    Space Invaders arcade machine emulator

    Copyright (c) 1997-2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef INVADERS_H_
#define INVADERS_H_

#include <math.h>

#include <emu/emu_standard_machine.h>
#include <emu/emu_memory_iostream.h>
#include <emu/emu_sample_player.h>
#include <cpu/i8080.h>
#include <sound/rcfilter.h>
#include <sound/sn76477.h>
#include <sound/waveform.h>

#include <ase/ase_triangle_wave_vco1.h>
#include <ase/ase_noise.h>
#include <ase/ase_lowpass_filter.h>

class T555Astable
{
public:
    T555Astable( double ra, double rb, double c ) {
        setParameters( ra, rb, c );
        active_ = false;
    }

    void setVariableResistor( double ra ) {
        setParameters( ra, rb_, c_ );
    }

    void setParameters( double ra, double rb, double c ) {
        c_ = c;
        ra_ = ra;
        rb_ = rb;

        double t0 = log(3.0) * c * (ra + rb);
        double t1 = log(2.0) * c * rb;
        double t2 = log(2.0) * c * (ra + rb);

        wfg_.setHiLoPeriod( t2, t1, t0 );
    }

    void setAmplitude( int hi, int lo = 0 ) {
        wfg_.setAmplitude( hi, lo );
    }

    bool active() const {
        return active_;
    }

    void setActive( bool active ) {
        if( active_ != active ) {
            if( ! active_ ) {
                wfg_.reset();
            }
            active_ = active;
        }
    }

    void andWithBuffer( int * data, unsigned len, unsigned samplingRate ) {
        if( active_ ) {
            wfg_.andWithBuffer( data, len, samplingRate );
        }
    }

    void setBuffer( int * data, unsigned len, unsigned samplingRate ) {
        if( active_ ) {
            wfg_.setBuffer( data, len, samplingRate );
        }
    }

private:
    double ra_;
    double rb_;
    double c_;
    bool active_;
    TSquareWaveGenerator wfg_;
};

struct SpaceInvadersBoard : public I8080Environment
{
    SpaceInvadersBoard();

    ~SpaceInvadersBoard();

    virtual void run();
    virtual void reset();
    
    void setVram( unsigned char * vram ) {
        vram_ = vram;
    }

    // Implementation of the I8080Environment interface
    unsigned char readByte( unsigned addr );
    void writeByte( unsigned, unsigned char );
    unsigned char readPort( unsigned port );
    void writePort( unsigned, unsigned char );

    // Member variables
    unsigned char   ram_[0x6000];
    unsigned char   port0i_;    // Port 0 in
    unsigned char   port1i_;    // Port 1 in
    unsigned char   port2i_;    // Port 2 in
    unsigned char   port2o_;    // Port 2 out
    unsigned char   port3o_;    // Port 3 out
    unsigned char   port4lo_;   // Port 4 out (lo)
    unsigned char   port4hi_;   // Port 4 out (hi)
    unsigned char   port5o_;    // Port 5 out
    unsigned char * vram_;
    unsigned char   color_prom_[2*1024];
    I8080 *         cpu_;
};

/**
    Space Invaders machine driver.

    @author Alessandro Scotti
    @version 1.2
*/
class SpaceInvaders : public TStandardMachine
{
public:
    virtual ~SpaceInvaders() {
        delete main_board_;
    }

    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );

    virtual void run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate );

    virtual void reset();

    static TMachine * createInstance() {
        return new SpaceInvaders;
    }

protected:
    SpaceInvaders( SpaceInvadersBoard * board = 0 );

    bool initialize( TMachineDriverInfo * info );

    void setActivePalette( unsigned char palette );

    void renderAnalogSounds( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate );

    void repaintScreen();

    enum {
        ExtendPlayVibratoAmplitude = 30
    };

protected:
    unsigned            palette_data_[8][8];
    unsigned char       active_palette_;
    unsigned char       settings_;
    SpaceInvadersBoard *main_board_;
    TSamplePlayer       samples_[10];

    TMixerBuffer            walk_sound_buffer_;
    T555Astable             walk_timer_;
    RCFilter                walk_rcfilter_1_;
    RCFilter                walk_rcfilter_2_;

    TMixerBuffer            extend_play_sound_buffer_;
    T555Astable             extend_play_timer_;
    TSquareWaveGenerator    extend_play_tone_;
    unsigned                extend_play_vibrato_offset_;
    
    ATriangleWaveVCO        ufo_hit_;
    ATriangleWaveVCO        target_hit_;
    AWhiteNoise             shot_;
    int                     shot_active_;
    AWhiteNoise             flash_;
    ALowPassRCFilter        flash_rc1_;
    ALowPassRCFilter        flash_rc2_;
    int                     flash_active_;

    SN76477                 ufo_sound_;
};

class SpaceAttackII : public SpaceInvaders
{
public:
    static TMachine * createInstance() {
        return new SpaceAttackII;
    }

protected:
    SpaceAttackII();

    bool initialize( TMachineDriverInfo * info );
};

class OzmaWars : public SpaceInvaders
{
public:
    static TMachine * createInstance() {
        return new OzmaWars;
    }

protected:
    OzmaWars();

    bool initialize( TMachineDriverInfo * info );
};

class SpaceInvadersPartII : public SpaceInvaders
{
public:
    static TMachine * createInstance() {
        return new SpaceInvadersPartII;
    }

protected:
    SpaceInvadersPartII();

    bool initialize( TMachineDriverInfo * info );
};

class SpaceInvadersDeluxe : public SpaceInvadersPartII
{
public:
    static TMachine * createInstance() {
        return new SpaceInvadersDeluxe;
    }

protected:
    SpaceInvadersDeluxe();

    bool initialize( TMachineDriverInfo * info );
};

class LunarRescue : public SpaceInvaders
{
public:
    static TMachine * createInstance() {
        return new LunarRescue;
    }

protected:
    LunarRescue();

    bool initialize( TMachineDriverInfo * info );
};

struct RollingCrashBoard : public SpaceInvadersBoard
{
    RollingCrashBoard();

    virtual void run();

    unsigned char readByte( unsigned addr );
    void writeByte( unsigned, unsigned char );
    unsigned char readPort( unsigned port );
    void writePort( unsigned, unsigned char );
    
    // There is only 1K of color RAM but it is mapped on 13 address lines, of which 3 are ignored
    inline unsigned remapColorRamAddr( unsigned addr ) {
        return (addr & 0x1F) | ((addr & 0x1F00) >> 3);
    }
    
    unsigned char color_ram_[0x400];    // 1K
    unsigned char extra_ram_[0x1C00];
    unsigned char port0o_;
    bool port0o_written_;
    bool port1o_written_;
};

class RollingCrash : public SpaceInvaders
{
public:
    static TMachine * createInstance() {
        return new RollingCrash;
    }
    
protected:
    RollingCrash();
    
    bool initialize( TMachineDriverInfo * info );
};

#endif // INVADERS_H_
