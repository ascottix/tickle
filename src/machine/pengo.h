/*
    Pengo arcade machine emulator

    Copyright (c) 2006 Alessandro Scotti
*/
#ifndef PENGO_H_
#define PENGO_H_

#include <emu/emu_standard_machine.h>
#include <cpu/z80.h>
#include <sound/namcowsg3.h>

struct PengoBoard : public Z80Environment
{
    PengoBoard();

    ~PengoBoard();

    virtual void reset();

    virtual void run();

    // Implementation of the Z80Environment interface
    unsigned char readByte( unsigned addr );
    void writeByte( unsigned, unsigned char );

    void setOutputFlipFlop( unsigned char bit, unsigned char value );

    void setSoundPROM( const unsigned char * prom ) {
        sound_chip_.setSoundPROM( prom );
    }

    unsigned char   ram_[36*1024];          // ROM (32K) and RAM (4K)
    unsigned char   sprite_coords_[16];     // Sprite X/Y coordinates
    unsigned char   dsw0_;
    unsigned char   dsw1_;
    unsigned char   port0_;
    unsigned char   port1_;
    unsigned char   output_devices_;        // Output flip-flops set by the game program
    unsigned        coin_counter1_;
    unsigned        coin_counter2_;
    int             coin1_timer_;
    int             coin2_timer_;
    unsigned        char_bank_;
    unsigned        palette_bank_;
    unsigned        color_bank_;
    NamcoWsg3       sound_chip_;
    TWatchDog       watchdog_;
    Z80 *           cpu_;
};

/**
    Pengo machine driver.

    @author Alessandro Scotti
    @version 1.0
*/
class Pengo : public TStandardMachine
{
public:
    virtual ~Pengo();

    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );

    virtual void run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate );

    virtual void reset();

    static TMachine * createInstance() {
        return new Pengo( new PengoBoard );
    }

protected:
    virtual bool initialize( TMachineDriverInfo * info );

    Pengo( PengoBoard * );

    void decodeCharByte( unsigned char b, unsigned char * charbuf, int charx, int chary, int charwidth );
    void decodeCharLine( unsigned char * src, unsigned char * charbuf, int charx, int chary, int charwidth );
    void onVideoROMsChanged();
    TBitmapIndexed * renderVideo();

    PengoBoard * board() {
        return main_board_;
    }

    void setMonitorFlip( bool flipped ) {
        flip_monitor_ = flipped;
    }

private:
    Pengo( const Pengo & );
    Pengo & operator = ( const Pengo & );

    PengoBoard *        main_board_;
    bool                refresh_roms_;      // Set to true if ROM changed since last frame
    bool                flip_monitor_;
    unsigned            frame_counter_;     // How many times run() has been called since last reset() 
    unsigned char       video_rom_[16*1024]; // Video ROM (character and sprite data)    
    unsigned char       palette_prom_[32];
    unsigned char       color_prom_[4*256];
    // Internal tables and structures for faster access to data
    TBitBlock           char_data_;         // Character data for 2 banks of 256 8x8 characters
    TBitBlock           sprite_data_;       // Sprite data for 2 banks of 64 16x16 sprites
};

#endif // PENGO_H_
