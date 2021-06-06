/*
    Galaxian arcade machine emulator

    Copyright (c) 2004 Alessandro Scotti
*/
#ifndef GALAXIAN_H_
#define GALAXIAN_H_

#include <emu/emu_standard_machine.h>
#include <cpu/z80.h>

#include "galaxian_soundboard.h"

struct GalaxianMainBoard : public Z80Environment
{
    GalaxianMainBoard();

    ~GalaxianMainBoard() {
        delete cpu_;
    }

    void reset();

    void run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate );

    // Implementation of the Z80Environment interface
    unsigned char readByte( unsigned addr );
    void writeByte( unsigned, unsigned char );

    virtual void onWriteLatch9L( unsigned addr, unsigned char b );

    // Utilities
    void setOutputFlipFlop( unsigned char bit, unsigned char value );

    unsigned now();

    // Member variables
    unsigned char   rom_[16*1024];          // ROM
    unsigned char   ram_[ 2*1024];          // RAM
    unsigned char   video_ram_[0x400];      // Video RAM (2K)
    unsigned char   special_ram_[0x100];    // Special RAM (256) for sprites, bullets, etc.
    unsigned char   port0_;                 // IN0
    unsigned char   port1_;                 // IN1
    unsigned char   port2_;                 // IN2
    unsigned char   output_devices_;        // Output flip-flops set by the game program
    unsigned        coin_counter_;          // Coin meter
    unsigned char   bank_select_[3];        // Plane selection (a common extension of the original hardware)
    Z80 * cpu_;

    // Where things are mapped in memory
    unsigned    addr_rom_end_;
    unsigned    addr_ram_;
    unsigned    addr_ram_end_;
    unsigned    addr_video_ram_;
    unsigned    addr_video_ram_mirror_;
    unsigned    addr_special_ram_;
    unsigned    addr_port0_;
    unsigned    addr_port1_;
    unsigned    addr_port2_;
    unsigned    addr_watchdog_;
    unsigned    addr_latch_9l_;
    unsigned    addr_latch_9m_;
    unsigned    addr_interrupt_enable_;
    unsigned    addr_starfield_enable_;
    unsigned    addr_flip_x_;
    unsigned    addr_flip_y_;
    unsigned    addr_tone_pitch_;

    // Sound emulation
    TFrame *                current_frame_;
    unsigned                current_frame_size_;
    GalaxianSoundBoard      soundboard_;
};

class Galaxian : public TStandardMachine
{
public:
    virtual ~Galaxian() {
        delete main_board_;
    }

    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );

    virtual void run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate );

    virtual void reset();

    static TMachine * createInstance() {
        return new Galaxian( new GalaxianMainBoard );
    }

protected:
    Galaxian( GalaxianMainBoard * board );

    virtual bool initialize( TMachineDriverInfo * info );

    void decodeChar( const unsigned char * src, TBitBlock * bb, int ox, int oy );
    void decodeSprite( const unsigned char * src, TBitBlock * bb, int ox, int oy );
    void onVideoROMsChanged();
    TBitmapIndexed * renderVideo();

protected:
    enum {
        NumOfStars = 252*10,
        BackgroundPaletteIndex = 32, // Just after the PROM colors
        BulletPaletteIndex = BackgroundPaletteIndex + 1,
        StarsPaletteIndex = BulletPaletteIndex + 2
    };

    struct StarfieldItem {
        int x;
        int y;
        int color;
    };

    virtual void drawStarfield();
    virtual unsigned translateCharCode( unsigned code );
    virtual unsigned translateSpriteCode( unsigned code );

    GalaxianMainBoard * main_board_;
    // Internal tables and structures for faster access to data
    bool          refresh_roms_;        // Set to true if ROM changed since last frame
    unsigned char video_rom_[8*1024];   // Video ROM (character and sprite data)    
    unsigned char palette_prom_[32];
    TBitBlock     char_data_;           // Character data for 256 8x8 characters
    TBitBlock     sprite_data_;         // Sprite data for 64 16x16 sprites
    // Not strictly part of original hardware, but very common modifications
    int           low_sprite_offset_;
    int           gfx_banks_;           // Allow an extra bank for sprites and characters
    unsigned      gfx_plane_size_;
    // Starfield emulation
    StarfieldItem starfield_[NumOfStars];   // Stars
    unsigned      starfield_blink_state_;   // Blink state
    unsigned      starfield_blink_timer_;   // Blink timer (microseconds)
    unsigned      starfield_blink_period_;  // Blink period (microseconds)
    int           starfield_scroll_pos_;
};

class WarOfBugs : public Galaxian
{
public:
    static TMachine * createInstance() {
        return new WarOfBugs();
    }

protected:
    WarOfBugs();

    virtual bool initialize( TMachineDriverInfo * info );
};

struct MoonCrestaMainBoard : public GalaxianMainBoard
{
    MoonCrestaMainBoard();

    virtual void onWriteLatch9L( unsigned addr, unsigned char b );
};

class MoonCresta : public Galaxian
{
public:
    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );

    static TMachine * createInstance() {
        return new MoonCresta();
    }

protected:
    MoonCresta();

    virtual bool initialize( TMachineDriverInfo * info );

    unsigned translateCharCode( unsigned code );
    unsigned translateSpriteCode( unsigned code );
};

struct UniWarSMainBoard : public GalaxianMainBoard
{
    virtual void onWriteLatch9L( unsigned addr, unsigned char b );
};

class UniWarS : public Galaxian
{
public:
    static TMachine * createInstance() {
        return new UniWarS();
    }

protected:
    UniWarS();

    virtual bool initialize( TMachineDriverInfo * info );

    unsigned translateCharCode( unsigned code );
    unsigned translateSpriteCode( unsigned code );
};

class Gingateikoku : public UniWarS
{
public:
    static TMachine * createInstance() {
        return new Gingateikoku();
    }

protected:
    Gingateikoku();

    virtual bool initialize( TMachineDriverInfo * info );
};

class BlackHole : public Galaxian
{
public:
    static TMachine * createInstance() {
        return new BlackHole();
    }

protected:
    BlackHole();

    virtual bool initialize( TMachineDriverInfo * info );
};

#endif // GALAXIAN_H_
