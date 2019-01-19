/*
    1942 arcade machine emulator

    Copyright (c) 2011 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef M1942_H_
#define M1942_H_

#include <emu/emu_standard_machine.h>
#include <cpu/z80.h>

#include "z80_ay3_soundboard.h"

struct M1942SoundBoard : public Z80_AY3_SoundBoard
{
    M1942SoundBoard();

    // Override a few methods to customize behavior
    virtual void run();
    unsigned char readPort( unsigned addr );
    void writePort( unsigned addr, unsigned char value );
    unsigned char onReadByte( unsigned addr );
    void onWriteByte( unsigned addr, unsigned char value );
    void onAfterSamplingStep( unsigned step );
    
    // Member variables
    unsigned timer_clock_;
    unsigned char sound_command_;
};

struct M1942MainBoard : public Z80Environment
{
    M1942MainBoard();

    ~M1942MainBoard() {
        delete cpu_;
    }

    void reset();

    void run();

    // Implementation of the Z80Environment interface
    unsigned char readByte( unsigned addr );
    void writeByte( unsigned, unsigned char );

    // Member variables
    unsigned char   rom_[0x14000];          // ROM
    unsigned char   ram_[0x1000];           // RAM (4K)
    unsigned char   video_ram_[0xC00];      // Video RAM (2K for foreground, 1K for background)
    unsigned char   sprite_ram_[0x80];      // Sprite RAM
    unsigned char * curr_rom_bank_;
    unsigned char   port0_;                 // IN0
    unsigned char   port1_;                 // IN1
    unsigned char   port2_;                 // IN2
    unsigned char   dsw_a_;                 // DSW A
    unsigned char   dsw_b_;                 // DSW B
    unsigned char   sound_command_;
    bool            sound_reset_;
    unsigned        palette_bank_;
    int             scroll_;
    Z80 * cpu_;
};

class M1942 : public TStandardMachine
{
public:
    virtual ~M1942() {
        delete main_board_;
    }

    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );

    virtual void run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate );

    virtual void reset();

    static TMachine * createInstance() {
        return new M1942( new M1942MainBoard );
    }

protected:
    M1942( M1942MainBoard * board );

    virtual bool initialize( TMachineDriverInfo * info );

    void onVideoROMsChanged();
    TBitmapIndexed * renderVideo();

protected:
    M1942SoundBoard  sound_board_;
    M1942MainBoard * main_board_;
    bool          refresh_roms_;        // Set to true if ROM changed since last frame
    unsigned char char_rom_[0x2000];    // Char ROM
    unsigned char tile_rom_[0xC000];    // Tile ROM
    unsigned char sprite_rom_[0x10000]; // Sprite ROM
    unsigned char palette_lookup_char_prom_[0x100];
    unsigned char palette_lookup_tile_prom_[0x100];
    unsigned char palette_lookup_sprite_prom_[0x100];
    unsigned char palette_prom_[0x300];
    unsigned char palette_char_[256];
    unsigned char palette_tile_[4][256];
    unsigned char palette_sprite_[256];
    TBitBlock     char_data_;           // Character data for 512 8x8 characters (foreground)
    TBitBlock     tile_data_;           // Tile data for 512 16x16 tiles (background)
    TBitBlock     sprite_data_;         // Sprite data for 64 16x16 sprites
};

#endif // M1942_H_
