/*
    Galaga arcade machine emulator

    Copyright (c) 2011 Alessandro Scotti
*/
#ifndef GALAGA_H_
#define GALAGA_H_

#include <emu/emu_standard_machine.h>
#include <cpu/z80.h>
#include <sound/namcowsg3.h>

#include <ase/ase_bandpass_filter.h>
#include <ase/ase_clipper.h>
#include <ase/ase_inverter.h>
#include <ase/ase_latch.h>
#include <ase/ase_noise.h>
#include <ase/ase_capacitor_with_switch.h>

#include "namco05.h"
#include "namco51.h"

struct GalagaMainBoard;

struct GalagaSoundBoard : public Z80Environment
{
    GalagaSoundBoard(GalagaMainBoard *);
    
    ~GalagaSoundBoard();
    
    // Implementation of the Z80Environment interface
    unsigned char readByte( unsigned addr );
    void writeByte( unsigned, unsigned char );
    
    // Member variables
    unsigned char   rom_[8*1024];           // ROM
    GalagaMainBoard * main_board_;
    Z80 * cpu_;
};

struct GalagaAuxBoard : public Z80Environment
{
    GalagaAuxBoard(GalagaMainBoard *);
    
    ~GalagaAuxBoard();
    
    // Implementation of the Z80Environment interface
    unsigned char readByte( unsigned addr );
    void writeByte( unsigned, unsigned char );
    
    // Member variables
    unsigned char   rom_[8*1024];           // ROM
    GalagaMainBoard * main_board_;
    Z80 * cpu_;
};

struct GalagaMainBoard : public Z80Environment
{
    GalagaMainBoard();
    
    ~GalagaMainBoard();
    
    void run();
    
    void reset();
    
    // Implementation of the Z80Environment interface
    unsigned char readByte( unsigned addr );
    void writeByte( unsigned, unsigned char );
    
    // Member variables
    unsigned char   rom_[16*1024];           // ROM
    unsigned char   video_ram_[2*1024];
    unsigned char   ram1_[1*1024];
    unsigned char   ram2_[1*1024];
    unsigned char   ram3_[1*1024];
    bool            cpu1_int_enabled_;
    bool            cpu2_int_enabled_;
    bool            cpu3_int_enabled_;
    bool            halt_cpu23_;
    Z80 *           cpu_2_;
    Z80 *           cpu_3_;
    GalagaAuxBoard * aux_board_;
    GalagaSoundBoard * sound_board_;
    unsigned char   dsw_a_;
    unsigned char   dsw_b_;
    unsigned char   port_0_;
    unsigned char   port_1_;
    unsigned char   namco06xx_control_;
    unsigned        namco06xx_nmi_counter_;
    Namco05xx       namco05xx;
    Namco51xx       namco51xx;
    NamcoWsg3       sound_chip_;
    unsigned        frame_count_;
    unsigned char   cheat_;
    Z80 * cpu_;
    
    // Explosion
    AWhiteNoise * a_noise_;
    ALatch * a_hit_latch_;
    AClipperLo * a_hit_diode_;
    ACapacitorWithSwitch * a_hit_;
    AActiveBandPassFilter * a_hit_filter_;
};

class Galaga : public TStandardMachine
{
public:
    virtual ~Galaga();
    
    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );
    
    virtual void run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate );
    
    virtual void reset();
    
    static TMachine * createInstance() {
        return new Galaga();
    }
    
protected:
    Galaga();
    
    virtual bool initialize( TMachineDriverInfo * info );
    
    void onVideoROMsChanged();
    TBitmapIndexed * renderVideo();
    
protected:
    GalagaMainBoard * main_board_;
    bool          refresh_roms_;        // Set to true if ROM changed since last frame
    unsigned char video_rom_[0x3000];
    unsigned char palette_prom_[0x20];
    unsigned char palette_lookup_char_prom_[0x100];
    unsigned char palette_lookup_sprite_prom_[0x100];
    unsigned char palette_char_[256];
    unsigned char palette_sprite_[256];
    TBitBlock     char_data_;           // Character data for 256 8x8 characters
    TBitBlock     sprite_data_;         // Sprite data for 128 16x16 sprites
};

#endif // GALAGA_H_
