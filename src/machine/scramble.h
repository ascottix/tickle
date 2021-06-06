/*
    Scramble arcade machine emulator

    Copyright (c) 2004 Alessandro Scotti
*/
#ifndef SCRAMBLE_H_
#define SCRAMBLE_H_

#include <emu/emu_standard_machine.h>
#include <cpu/z80.h>

#include "z80_ay3_soundboard.h"

struct ScrambleSoundBoard : public Z80_AY3_SoundBoard
{
    ScrambleSoundBoard();

    // Override a few methods to customize behavior
    virtual void run();
    unsigned char readPort( unsigned addr );
    void writePort( unsigned addr, unsigned char value );

    // Member variables
    unsigned timer_clock_;
};

struct ScrambleMainBoard : public Z80Environment
{
    ScrambleMainBoard();

    ~ScrambleMainBoard() {
        delete cpu_;
    }

    void reset();

    void run();

    // Implementation of the Z80Environment interface
    unsigned char readByte( unsigned addr );
    void writeByte( unsigned, unsigned char );

    // Utilities
    void setOutputFlipFlop( unsigned char bit, unsigned char value );

    // Member variables
    unsigned char   ram_[0x50FF] ;          // ROM (16K), RAM (4K) and "special" RAM (sprites, stars, bullets)
    unsigned char   port0_;                 // IN0
    unsigned char   port1_;                 // IN1
    unsigned char   port2_;                 // IN2
    unsigned char   output_devices_;        // Output flip-flops set by the game program
    unsigned char   sound_command_;
    unsigned        coin_counter_;          // Coin meter
    Z80 * cpu_;
};

class Scramble : public TStandardMachine
{
public:
    virtual ~Scramble() {
        delete main_board_;
    }

    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );

    virtual void run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate );

    virtual void reset();

    static TMachine * createInstance() {
        return new Scramble( new ScrambleMainBoard );
    }

protected:
    Scramble( ScrambleMainBoard * board );

    virtual bool initialize( TMachineDriverInfo * info );

    void decodeChar( const unsigned char * src, TBitBlock * bb, int ox, int oy, int planes = 2, unsigned plane_size = 0x800 );
    void decodeSprite( const unsigned char * src, TBitBlock * bb, int ox, int oy );
    void onVideoROMsChanged();
    TBitmapIndexed * renderVideo();

protected:
    enum {
        NumOfStars = 252,
        BackgroundPaletteIndex = 32, // Just after the PROM colors
        BulletPaletteIndex = BackgroundPaletteIndex + 1,
        StarsPaletteIndex = BulletPaletteIndex + 1
    };

    struct StarfieldItem {
        int x;
        int y;
        int color;
    };

    virtual void drawStarfield();
    virtual void drawBullets( unsigned char * bullet_ram, unsigned mode );

    ScrambleSoundBoard  sound_board_;
    ScrambleMainBoard * main_board_;
    // Internal tables and structures for faster access to data
    bool          refresh_roms_;        // Set to true if ROM changed since last frame
    unsigned char video_rom_[4*1024];   // Video ROM (character and sprite data)    
    unsigned char palette_prom_[32];
    TBitBlock     char_data_;           // Character data for 256 8x8 characters
    TBitBlock     sprite_data_;         // Sprite data for 64 16x16 sprites
    // Starfield emulation
    StarfieldItem starfield_[NumOfStars];   // Stars
    unsigned      starfield_blink_state_;   // Blink state
    unsigned      starfield_blink_timer_;   // Blink timer (microseconds)
    unsigned      starfield_blink_period_;  // Blink period (microseconds)
    int           starfield_scroll_pos_;
};

class AmidarOnScramble : public Scramble
{
public:
    static TMachine * createInstance() {
        return new AmidarOnScramble;
    }

protected:
    AmidarOnScramble();

    virtual bool initialize( TMachineDriverInfo * info );
};

class Atlantis : public Scramble
{
public:
    static TMachine * createInstance() {
        return new Atlantis;
    }

protected:
    Atlantis();

    virtual bool initialize( TMachineDriverInfo * info );
};

class TheEnd : public Scramble
{
public:
    static TMachine * createInstance() {
        return new TheEnd;
    }

protected:
    TheEnd();

    virtual bool initialize( TMachineDriverInfo * info );

    virtual void drawStarfield();
    virtual void drawBullets( unsigned char * bullet_ram, unsigned mode );
};

struct MarsMainBoard : public ScrambleMainBoard
{
    // Mars has different port mappings and needs to override the Z80Environment interface
    unsigned char readByte( unsigned addr );
    void writeByte( unsigned addr, unsigned char value );

    unsigned char port3_;
};

class Mars : public Scramble
{
public:
    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );

    static TMachine * createInstance() {
        return new Mars;
    }

protected:
    Mars();

    virtual bool initialize( TMachineDriverInfo * info );
};

#endif // SCRAMBLE_H_
