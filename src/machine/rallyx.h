/*
    Rally X arcade machine emulator

    Copyright (c) 2011 Alessandro Scotti
*/
#ifndef RALLY_X_H_
#define RALLY_X_H_

#include <emu/emu_standard_machine.h>
#include <cpu/z80.h>
#include <sound/namcowsg3.h>

#include <ase/ase_bandpass_filter.h>
#include <ase/ase_clipper.h>
#include <ase/ase_inverter.h>
#include <ase/ase_latch.h>
#include <ase/ase_noise.h>
#include <ase/ase_capacitor_with_switch.h>
#include <ase/ase_lowpass_filter.h>

struct RallyXMainBoard : public Z80Environment
{
    RallyXMainBoard();

    ~RallyXMainBoard() {
        delete cpu_;
    }

    void reset();

    void run();

    // Implementation of the Z80Environment interface
    unsigned char readByte( unsigned addr );
    void writeByte( unsigned, unsigned char );
    void writePort( unsigned, unsigned char );
    void onInterruptsEnabled();

    // Member variables
    unsigned char   rom_[0x4000];       // ROM
    unsigned char   ram_[0x800];        // RAM (2K)
    unsigned char   video_ram_[0x1000]; // Video RAM 
    unsigned char   radar_ram_[0x10];
    unsigned char   port0_;             // IN0
    unsigned char   port1_;             // IN1
    unsigned char   dsw_a_;             // DSW A
    unsigned char   irq_opcode_;
    bool            irq_enabled_;
    bool            irq_missed_;
    int             scroll_x_;
    int             scroll_y_;
    NamcoWsg3       sound_chip_;
    Z80 * cpu_;
    
    // Bang
    AWhiteNoise * a_noise_;
    ALatch * a_hit_latch_;
    AClipperLo * a_hit_diode_;
    ACapacitorWithSwitch * a_hit_;
    AActiveBandPassFilter * a_hit_filter_;
    ALowPassRCFilter * a_rc_filter_1_;
};

class RallyX : public TStandardMachine
{
public:
    virtual ~RallyX() {
        delete main_board_;
    }

    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );

    virtual void run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate );

    virtual void reset();

    static TMachine * createInstance() {
        return new RallyX( new RallyXMainBoard );
    }

protected:
    RallyX( RallyXMainBoard * board );

    virtual bool initialize( TMachineDriverInfo * info );

    void onVideoROMsChanged();
    TBitmapIndexed * renderVideo();

protected:
    RallyXMainBoard * main_board_;
    bool          refresh_roms_;        // Set to true if ROM changed since last frame
    unsigned char video_rom_[0x1000];
    unsigned char palette_lookup_prom_[0x100];
    unsigned char palette_xlat_[0x100];
    unsigned char palette_prom_[0x20];
    unsigned char dots_prom_[0x100];
    TBitBlock     char_data_;       // Character data for 256 8x8 characters
    TBitBlock     dots_data_;       // Character data for 8 4x4 radar dots
    TBitBlock     sprite_data_;     // Sprite data for 64 16x16 sprites
};

class NewRallyX : public RallyX
{
public:
    static TMachine * createInstance() {
        return new NewRallyX( new RallyXMainBoard );
    }
    
    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );
    
protected:
    virtual bool initialize( TMachineDriverInfo * info );
    
    NewRallyX( RallyXMainBoard * board );
};

#endif // RALLY_X_H_
