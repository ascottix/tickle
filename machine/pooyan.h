/*
    Pooyan arcade machine emulator

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef POOYAN_H_
#define POOYAN_H_

#include <emu/emu_standard_machine.h>
#include <cpu/z80.h>

#include "z80_ay3_soundboard.h"

struct PooyanSoundBoard : public Z80_AY3_SoundBoard
{
    PooyanSoundBoard();

    // Override a few methods to customize behavior
    virtual void run();
    unsigned char onReadByte( unsigned addr );
    void onWriteByte( unsigned addr, unsigned char value );

    // Member variables
    unsigned timer_clock_;
};

struct PooyanMainBoard : public Z80Environment
{
    PooyanMainBoard( PooyanSoundBoard * sound_board );

    ~PooyanMainBoard() {
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
    unsigned char   ram_[38*1024];          // ROM (32K) and RAM (6K)
    unsigned char   dip_switches_1_;        // DSW1
    unsigned char   dip_switches_2_;        // DSW2
    unsigned char   port0_;                 // IN0
    unsigned char   port1_;                 // IN1
    unsigned char   port2_;                 // IN2
    unsigned char   output_devices_;        // Output flip-flops set by the game program
    unsigned        coin_counter_1_;        // Coin meter for slot 1
    unsigned        coin_counter_2_;        // Coin meter for slot 2
    unsigned        frame_counter_;         // How many times run() has been called since last reset() 
    Z80 * cpu_;
    PooyanSoundBoard * sound_board_;
};

/**
    Pooyan machine driver.

    @author Alessandro Scotti
    @version 1.1
*/
class Pooyan : public TStandardMachine
{
public:
    virtual ~Pooyan() {
        delete main_board_;
    }

    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );

    virtual void run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate );

    virtual void reset();

    static TMachine * createInstance() {
        return new Pooyan;
    }

protected:
    Pooyan();

    virtual bool initialize( TMachineDriverInfo * info );

    void decodeChar( const unsigned char * src, TBitBlock * bb, int ox, int oy );
    void onVideoROMsChanged();
    TBitmapIndexed * renderVideo();

private:
    bool                refresh_roms_;      // Set to true if ROM changed since last frame
    PooyanSoundBoard    sound_board_;
    PooyanMainBoard *   main_board_;
    // Internal tables and structures for faster access to data
    unsigned char       video_rom_[16*1024];    // Video ROM (character and sprite data)    
    unsigned char       palette_prom_[32];
    unsigned char       sprite_table_prom_[256];
    unsigned char       char_table_prom_[256];
    unsigned char       char_xlat_table_[256];  // Char color lookup table
    unsigned char       sprite_xlat_table_[256];// Sprite color lookup table
    TBitBlock           char_data_;             // Character data for 256 8x8 characters
    TBitBlock           sprite_data_;           // Sprite data for 64 16x16 sprites
};

#endif // POOYAN_H_
