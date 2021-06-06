/*
    Frogger arcade machine emulator

    Copyright (c) 2004 Alessandro Scotti
*/
#ifndef FROGGER_H_
#define FROGGER_H_

#include <emu/emu_standard_machine.h>
#include <cpu/z80.h>

#include "z80_ay3_soundboard.h"

struct FroggerSoundBoard : public Z80_AY3_SoundBoard
{
    FroggerSoundBoard();

    // Override a few methods to customize behavior
    virtual void run();
    unsigned char readPort( unsigned addr );
    void writePort( unsigned addr, unsigned char value );

    // Member variables
    unsigned timer_clock_;
};

struct FroggerMainBoard : public Z80Environment
{
    FroggerMainBoard();

    ~FroggerMainBoard() {
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
    unsigned char   rom_[0x4000] ;          // ROM (16K)
    unsigned char   ram_[0x800];            // RAM (2K)
    unsigned char   video_ram_[0x400];      // Video RAM (1K)
    unsigned char   special_ram_[0x100];    // Special RAM (sprites, attributes)
    unsigned char   port0_;                 // IN0
    unsigned char   port1_;                 // IN1
    unsigned char   port2_;                 // IN2
    unsigned char   output_devices_;        // Output flip-flops set by the game program
    unsigned char   sound_command_;
    unsigned        coin_counter_1_;        // Coin meter 1
    unsigned        coin_counter_2_;        // Coin meter 2
    Z80 * cpu_;
};

class Frogger : public TStandardMachine
{
public:
    virtual ~Frogger() {
        delete main_board_;
    }

    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );

    virtual void run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate );

    virtual void reset();

    static TMachine * createInstance() {
        return new Frogger( new FroggerMainBoard );
    }

protected:
    Frogger( FroggerMainBoard * board );

    virtual bool initialize( TMachineDriverInfo * info );

    void decodeChar( const unsigned char * src, TBitBlock * bb, int ox, int oy, int planes = 2, unsigned plane_size = 0x800 );
    void decodeSprite( const unsigned char * src, TBitBlock * bb, int ox, int oy );
    void onVideoROMsChanged();
    TBitmapIndexed * renderVideo();

protected:
    enum {
        BackgroundPaletteIndex = 32, // Just after the PROM colors
    };

    FroggerSoundBoard  sound_board_;
    FroggerMainBoard * main_board_;
    // Internal tables and structures for faster access to data
    bool          refresh_roms_;        // Set to true if ROM changed since last frame
    unsigned char video_rom_[4*1024];   // Video ROM (character and sprite data)    
    unsigned char palette_prom_[32];
    TBitBlock     char_data_;           // Character data for 256 8x8 characters
    TBitBlock     sprite_data_;         // Sprite data for 64 16x16 sprites
};

#endif // FROGGER_H_
