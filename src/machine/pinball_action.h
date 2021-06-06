/*
    Pinball Action arcade machine emulator

    Copyright (c) 2004 Alessandro Scotti
*/
#ifndef PINBALL_ACTION_H_
#define PINBALL_ACTION_H_

#include <emu/emu_standard_machine.h>
#include <cpu/z80.h>
#include <sound/ay-3-8910.h>

struct PinballActionSoundBoard : public Z80Environment
{
    PinballActionSoundBoard();

    ~PinballActionSoundBoard();

    void run( int interrupt );
    void reset();

    // Implementation of the Z80Environment interface
    unsigned char readByte( unsigned addr );
    void writeByte( unsigned addr, unsigned char value );
    void writePort( unsigned addr, unsigned char value );
    void onInterruptsEnabled();
    
    void triggerInterrupt( unsigned vector );

    // Member variables
    unsigned char rom_[8*1024]; // ROM (8K)
    unsigned char ram_[2*1024]; // RAM (2K)
    unsigned char command_;
    unsigned interrupt_pending_;
    Z80 * cpu_;
    AY_3_8910 sound_chip_[3];
};

struct PinballActionMainBoard : public Z80Environment
{
    PinballActionMainBoard( PinballActionSoundBoard * sound_board );

    ~PinballActionMainBoard() {
        delete cpu_;
    }

    void reset();

    // Implementation of the Z80Environment interface
    unsigned char readByte( unsigned addr );
    void writeByte( unsigned, unsigned char );

    // Utilities
    void setOutputFlipFlop( unsigned char bit, unsigned char value );

    // Member variables
    unsigned char   rom_[40*1024];          // ROM
    unsigned char   ram_[0x2600];           // RAM (9.5K, includes sprites and video)
    unsigned char   dip_switches_0_;        // DSW0
    unsigned char   dip_switches_1_;        // DSW1
    unsigned char   port0_;                 // IN0
    unsigned char   port1_;                 // IN1
    unsigned char   port2_;                 // IN2
    unsigned char   output_devices_;        // Output flip-flops set by the game program
    int             shake_;
    Z80 * cpu_;
    PinballActionSoundBoard * sound_board_;
};

/**
    Pinball Action machine driver.

    @author Alessandro Scotti
    @version 1.0
*/
class PinballAction : public TStandardMachine
{
public:
    virtual ~PinballAction() {
    }

    virtual bool initialize( TMachineDriverInfo * info );

    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );

    virtual void run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate );

    virtual void reset();

    static TMachine * createInstance() {
        return new PinballAction;
    }

protected:
    PinballAction();

    void decodeChar( const unsigned char * src, TBitBlock * bb, int ox, int oy, int planes = 3, unsigned plane_size = 0x2000 );
    void decodeSprite( const unsigned char * src, TBitBlock * bb, int ox, int oy );
    void onVideoROMsChanged();
    TBitmapIndexed * renderVideo();

private:
    bool refresh_roms_; // True if ROM changed since last frame
    PinballActionSoundBoard sound_board_;
    PinballActionMainBoard main_board_;
    // Internal tables and structures for faster access to data
    unsigned char   video_rom_s_[24*1024];
    unsigned char   video_rom_j_[64*1024];
    unsigned char   video_rom_b_[24*1024];
    bool            fg_char_null_[1024]; // True if corresponding foreground char is empty (i.e. fully transparent)
    TBitBlock       bg_char_data_;
    TBitBlock       fg_char_data_;
    TBitBlock       sm_sprite_data_;
    TBitBlock       lg_sprite_data_;
};

#endif // PINBALL_ACTION_H_
