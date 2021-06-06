/*
    Pacman arcade machine emulator

    Copyright (c) 1997-2003,2004 Alessandro Scotti
*/
#ifndef PACMAN_H_
#define PACMAN_H_

#include <emu/emu_standard_machine.h>
#include <cpu/z80.h>
#include <sound/namcowsg3.h>

struct PacmanBoard : public Z80Environment
{
    PacmanBoard();

    ~PacmanBoard();

    virtual void reset();

    virtual void run();

    // Implementation of the Z80Environment interface
    unsigned char readByte( unsigned addr );
    void writeByte( unsigned, unsigned char );
    void writePort( unsigned, unsigned char );

    void setOutputFlipFlop( unsigned char bit, unsigned char value );

    void setSoundPROM( const unsigned char * prom ) {
        sound_chip_.setSoundPROM( prom );
    }

    unsigned char   ram_[20*1024];          // ROM (16K) and RAM (4K)
    unsigned char   sprite_coords_[32];     // Sprite X/Y coordinates
    unsigned char   dip_switches_;
    unsigned char   port1_;
    unsigned char   port2_;
    unsigned char   output_devices_;        // Output flip-flops set by the game program
    unsigned char   interrupt_vector_;
    unsigned        coin_counter_;
    NamcoWsg3       sound_chip_;
    TWatchDog       watchdog_;
    Z80 *           cpu_;
};

/**
    Puckman machine driver.

    @author Alessandro Scotti
    @version 1.1
*/
class Puckman : public TStandardMachine
{
public:
    virtual ~Puckman();

    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );

    virtual void run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate );

    virtual void reset();

    static TMachine * createInstance() {
        return new Puckman( new PacmanBoard );
    }

protected:
    virtual bool initialize( TMachineDriverInfo * info );

    Puckman( PacmanBoard * );

    void decodeCharByte( unsigned char b, unsigned char * charbuf, int charx, int chary, int charwidth );
    void decodeCharLine( unsigned char * src, unsigned char * charbuf, int charx, int chary, int charwidth );
    void onVideoROMsChanged();
    TBitmapIndexed * renderVideo();

    PacmanBoard * board() {
        return main_board_;
    }

    void setMonitorFlip( bool flipped ) {
        flip_monitor_ = flipped;
    }

private:
    Puckman( const Puckman & );
    Puckman & operator = ( const Puckman & );

    PacmanBoard *       main_board_;
    bool                refresh_roms_;      // Set to true if ROM changed since last frame
    bool                flip_monitor_;
    unsigned            frame_counter_;     // How many times run() has been called since last reset() 
    unsigned char       video_rom_[8*1024]; // Video ROM (character and sprite data)    
    unsigned char       palette_prom_[32];
    unsigned char       color_prom_[256];
    // Internal tables and structures for faster access to data
    TBitBlock           char_data_;         // Character data for 256 8x8 characters
    TBitBlock           sprite_data_;       // Sprite data for 64 16x16 sprites
};

/** Pacman machine driver. */
class Pacman : public Puckman
{
public:
    static TMachine * createInstance() {
        return new Pacman( new PacmanBoard );
    }

protected:
    virtual bool initialize( TMachineDriverInfo * info );

    Pacman( PacmanBoard * board );
};

/** Pacman Plus machine driver. */
class PacmanPlus : public Puckman
{
public:
    static TMachine * createInstance() {
        return new PacmanPlus;
    }

    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );

protected:
    virtual bool initialize( TMachineDriverInfo * info );

    PacmanPlus();
};

struct MsPacmanBoard : public PacmanBoard
{
    // Implementation of the Z80Environment interface
    unsigned char readByte( unsigned addr );
    void writeByte( unsigned, unsigned char );

    void onROMsChanged( unsigned char * encrypted_rom );

    unsigned char rom_aux_[26*1024];        // 10K new ROM plus 16K to backup old ROM (because it gets patched)
    unsigned char aux_board_enabled_;       // Whether the aux board has been enabled or not
};

/** Ms. Pacman machine driver. */
class MsPacman : public Pacman
{
public:
    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );

    virtual void run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate );

    static TMachine * createInstance() {
        return new MsPacman( new MsPacmanBoard );
    }

protected:
    virtual bool initialize( TMachineDriverInfo * info );

    MsPacman( MsPacmanBoard * board );

private:
    bool refresh_roms_;
    MsPacmanBoard * main_board_;
    unsigned char encrypted_rom_[10*1024];
};

struct CrushRollerBoard : public PacmanBoard
{
    unsigned char readByte( unsigned addr );
    void writeByte( unsigned, unsigned char );
};

class CrushRoller : public Puckman
{
public:
    static TMachine * createInstance() {
        return new CrushRoller();
    }

protected:
    virtual bool initialize( TMachineDriverInfo * info );

    CrushRoller();
};


class MakeTrax : public CrushRoller
{
public:
    static TMachine * createInstance() {
        return new MakeTrax();
    }

protected:
    virtual bool initialize( TMachineDriverInfo * info );

    MakeTrax();
};

class Eyes : public Puckman
{
public:
    static TMachine * createInstance() {
        return new Eyes();
    }

    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );

protected:
    virtual bool initialize( TMachineDriverInfo * info );

    Eyes();
};

class JumpShot : public Puckman
{
public:
    static TMachine * createInstance() {
        return new JumpShot();
    }

    virtual bool setResourceFile( int id, const unsigned char * buf, unsigned len );

protected:
    virtual bool initialize( TMachineDriverInfo * info );

    JumpShot();
};

#endif // PACMAN_H_
