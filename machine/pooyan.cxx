/*
    Pooyan arcade machine emulator

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "pooyan.h"

// Output flip-flops
enum {
    FlipScreen          = 0x01,
    CoinMeter1          = 0x02,
    CoinMeter2          = 0x04,
    InterruptEnabled    = 0x08,
    SoundInterrupt      = 0x10,
    HaveSoundInterrupt  = 0x20,
};

// Hardware info
enum {
    ScreenWidth             = 224,
    ScreenHeight            = 256,
    ScreenColors            = 256,
    ScreenWidthChars        = 28,
    ScreenHeightChars       = 32,
    VideoFrequency          = 60,
    CpuClock                = 3072000,
    SoundCpuClock           = 1789773,  // 14318180/8
    CpuCyclesPerFrame       = CpuClock / VideoFrequency,
    SoundCpuCyclesPerFrame  = SoundCpuClock / VideoFrequency
};

enum { 
    Ef4A, Ef5A, Ef6A, Ef7A, EfXX7A, EfXX8A, Ef10G, Ef9G, Ef9A, Ef8A, EfPR1, EfPR2, EfPR3,
    OptLives, OptCabinet, OptBonus, OptDifficulty, OptSounds, OptRackTest, OptCoinA, OptCoinB,
};

// Machine info
static TMachineInfo PooyanInfo = { 
    "pooyan", "Pooyan", S_Konami, 1982, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TGameRegistrationHandler reg( &PooyanInfo, Pooyan::createInstance );

bool Pooyan::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    // Need to refresh some internal tables if the video/color ROMs change
    refresh_roms_ |= (id >= Ef10G);

    return 0 == resourceHandler()->handle( id, buf, len );
}

Pooyan::Pooyan() :
    char_data_( 8, 8*256 ),     // 256 8x8 characters
    sprite_data_( 16, 16*64 )   // 64 16x16 sprites
{
    main_board_ = new PooyanMainBoard( &sound_board_ );
    refresh_roms_ = false;

    createScreen( ScreenWidth, ScreenHeight, ScreenColors );

    eventHandler()->add( idCoinSlot1,        ptInverted, &main_board_->port0_, 0x01 );
    eventHandler()->add( idCoinSlot2,        ptInverted, &main_board_->port0_, 0x02 );
    eventHandler()->add( idKeyService1,      ptInverted, &main_board_->port0_, 0x04 );
    eventHandler()->add( idKeyStartPlayer1,  ptInverted, &main_board_->port0_, 0x08 );
    eventHandler()->add( idKeyStartPlayer2,  ptInverted, &main_board_->port0_, 0x10 );
    eventHandler()->add( idKeyP1Action1,     ptInverted, &main_board_->port1_, 0x10 );
    eventHandler()->add( idKeyP2Action1,     ptInverted, &main_board_->port2_, 0x10 );

    setJoystickHandler( 0, new TJoystickToPortHandler( idJoyP1Joystick1, ptInverted, jm4Way, &main_board_->port1_, 0x08040000 ) );
    setJoystickHandler( 1, new TJoystickToPortHandler( idJoyP2Joystick1, ptInverted, jm4Way, &main_board_->port2_, 0x08040000 ) );

    registerDriver( PooyanInfo );
}

bool Pooyan::initialize( TMachineDriverInfo * info )
{
    // Declare resources
    resourceHandler()->add( Ef4A,       "1.4a",   0x2000, efROM,      main_board_->ram_+0x0000 );
    resourceHandler()->add( Ef5A,       "2.5a",   0x2000, efROM,      main_board_->ram_+0x2000 );
    resourceHandler()->add( Ef6A,       "3.6a",   0x2000, efROM,      main_board_->ram_+0x4000 );
    resourceHandler()->add( Ef7A,       "4.7a",   0x2000, efROM,      main_board_->ram_+0x6000 );
    resourceHandler()->add( EfXX7A,    "xx.7a",   0x1000, efROM,      sound_board_.rom()+0x0000 );
    resourceHandler()->add( EfXX8A,    "xx.8a",   0x1000, efROM,      sound_board_.rom()+0x1000 );
    
    resourceHandler()->add( Ef10G,      "8.10g",  0x1000, efVideoROM, video_rom_+0x0000 );
    resourceHandler()->add( Ef9G,       "7.9g",   0x1000, efVideoROM, video_rom_+0x1000 );
    resourceHandler()->add( Ef9A,       "6.9a",   0x1000, efVideoROM, video_rom_+0x2000 );
    resourceHandler()->add( Ef8A,       "5.8a",   0x1000, efVideoROM, video_rom_+0x3000 );
    
    resourceHandler()->add( EfPR1, "pooyan.pr1",  0x20, efPalettePROM, palette_prom_ );
    
    resourceHandler()->add( EfPR2, "pooyan.pr2",  0x100, efColorPROM, sprite_table_prom_ );
    resourceHandler()->add( EfPR3, "pooyan.pr3",  0x100, efColorPROM, char_table_prom_ );

    resourceHandler()->addToMachineDriverInfo( info );

    // Setup user interface
    TUserInterface * ui = info->userInterface();
    TUiOption * option;
    TUiOptionGroup * group = ui->addGroup( S_DSW_1, dtDipSwitch );

    option = group->add( OptLives, "Lives per game", 2, 0x03 );
    option->add( "5", 0x00 );
    option->add( "4", 0x01 );
    option->add( "3", 0x02 );
    option->add( S_FreePlay, 0x03 );

    option = group->add( OptCabinet, S_Cabinet, 0, 0x04 );
    option->add( S_Upright, 0x00 );
    option->add( S_Cocktail, 0x04 );

    option = group->add( OptBonus, "Bonus life at", 0, 0x08 );
    option->add( "30000/70000", 0x00 );
    option->add( "50000/80000", 0x08 );

    option = group->add( OptDifficulty, "Difficulty", 7, 0x70 );
    option->add( S_Hardest, 0x00 );
    option->add( S_Harder, 0x10 );
    option->add( S_Hard, 0x20 );
    option->add( S_Challenging, 0x30 );
    option->add( S_Normal, 0x40 );
    option->add( S_Easy, 0x50 );
    option->add( S_Easier, 0x60 );
    option->add( S_Easiest, 0x70 );

    option = group->add( OptSounds, "Demo Sounds", 0, 0x80 );
    option->add( S_On, 0x00 );
    option->add( S_Off, 0x80 );

    option = group->add( OptCoinA, "Coin A", 0, 0x0F );
    option->add( S_1Coin1Play, 0x0F );
    option->add( S_1Coin2Play, 0x0E );
    option->add( S_1Coin3Play, 0x0D );
    option->add( S_1Coin4Play, 0x0C );
    option->add( S_1Coin5Play, 0x0B );
    option->add( S_1Coin6Play, 0x0A );
    option->add( S_1Coin7Play, 0x09 );
    option->add( S_2Coin1Play, 0x08 );
    option->add( S_2Coin3Play, 0x07 );
    option->add( S_2Coin5Play, 0x06 );
    option->add( S_3Coin1Play, 0x05 );
    option->add( S_3Coin2Play, 0x04 );
    option->add( S_3Coin4Play, 0x03 );
    option->add( S_4Coin1Play, 0x02 );
    option->add( S_4Coin3Play, 0x01 );
    option->add( S_FreePlay, 0x00 );

    option = group->add( OptCoinB, "Coin B", 0, 0xF0 );
    option->add( S_1Coin1Play, 0xF0 );
    option->add( S_1Coin2Play, 0xE0 );
    option->add( S_1Coin3Play, 0xD0 );
    option->add( S_1Coin4Play, 0xC0 );
    option->add( S_1Coin5Play, 0xB0 );
    option->add( S_1Coin6Play, 0xA0 );
    option->add( S_1Coin7Play, 0x90 );
    option->add( S_2Coin1Play, 0x80 );
    option->add( S_2Coin3Play, 0x70 );
    option->add( S_2Coin5Play, 0x60 );
    option->add( S_3Coin1Play, 0x50 );
    option->add( S_3Coin2Play, 0x40 );
    option->add( S_3Coin4Play, 0x30 );
    option->add( S_4Coin1Play, 0x20 );
    option->add( S_4Coin3Play, 0x10 );
    option->add( "Attract mode (no play)", 0x00 );

    group = ui->addGroup( "Board Settings", dtBoardSwitch );

    option = group->add( OptRackTest, "Rack mode", 0, 0x10 );
    option->add( S_Normal, 0x10 );
    option->add( S_Test, 0x00 );

    // Setup the option handler
    optionHandler()->add( &main_board_->dip_switches_1_, ui, OptLives, OptCabinet, OptBonus, OptDifficulty, OptSounds );
    optionHandler()->add( &main_board_->dip_switches_2_, ui, OptCoinA, OptCoinB );
    optionHandler()->add( &main_board_->port0_, ui, OptRackTest );

    // Ask notification so internal variables are same as GUI
    return true;
}

void Pooyan::reset()
{
    main_board_->reset();
    sound_board_.reset();
}

void Pooyan::run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate )
{
    if( refresh_roms_ ) {
        onVideoROMsChanged();
        refresh_roms_ = false;
    }

    main_board_->run();

    if( main_board_->output_devices_ & HaveSoundInterrupt ) {
        sound_board_.interrupt( 0xFF );
    }

    sound_board_.run();

    sound_board_.playSound( SoundCpuCyclesPerFrame, frame->getMixer(), samplesPerFrame, samplingRate );

    frame->setVideo( renderVideo() );
}

void Pooyan::decodeChar( const unsigned char * src, TBitBlock * bb, int ox, int oy )
{
    int x = 0;
    int y = 0;

    for( int i=0; i<64; i++ ) {
        unsigned char mask = 1 << x;
        int pixel = 0;

        if( src[0x1000] & (mask << 4) ) pixel |= 0x04;
        if( src[0x1000] & (mask     ) ) pixel |= 0x08;
        if( src[0x0000] & (mask << 4) ) pixel |= 0x01;
        if( src[0x0000] & (mask     ) ) pixel |= 0x02;

        bb->setPixel( ox+y, oy + x + ((i < 32) ? 4 : 0), pixel ); // Note that x and y are inverted!

        x++;

        if( x == 4 ) {
            x = 0;
            y = (y + 1) & 7;
            src++;
        }
    }
}

TBitmapIndexed * Pooyan::renderVideo()
{
    unsigned char * video = main_board_->ram_ + 0x8400;
    unsigned char * color = main_board_->ram_ + 0x8000;

    // Draw the background first...
    TBltAddXlat         char_blitter_f( 0, char_xlat_table_ );
    TBltAddXlatReverse  char_blitter_r( 0, char_xlat_table_ );

    if( main_board_->output_devices_ & FlipScreen ) {
        // Flipped screen
        for( int y=0; y<ScreenHeightChars; y++ ) {
            for( int x=0; x<ScreenWidthChars; x++ ) {
                unsigned offset = 32*(ScreenWidthChars-1+2-x) + y;
                unsigned attr = color[offset];
                unsigned mode = ((attr & 0x40) ? 0 : opFlipY) | ((attr & 0x80) ? 0 : opFlipX);
                TBltAddXlat * blitter = (mode & opFlipX) ? &char_blitter_r : &char_blitter_f;

                screen()->bits()->copy( x*8, y*8, char_data_, 0, video[offset]*8, 8, 8, mode, blitter->color((attr & 0x0F)*16) );
            }
        }
    }
    else {
        // Normal screen
        for( int y=0; y<ScreenHeightChars; y++ ) {
            for( int x=0; x<ScreenWidthChars; x++ ) {
                unsigned offset = 32*(x+2) + (31-y);
                unsigned attr = color[offset];
                unsigned mode = ((attr & 0x40) ? opFlipY : 0) | ((attr & 0x80) ? opFlipX : 0);
                TBltAddXlat * blitter = (mode & opFlipX) ? &char_blitter_r : &char_blitter_f;

                screen()->bits()->copy( x*8, y*8, char_data_, 0, video[offset]*8, 8, 8, mode, blitter->color((attr & 0x0F)*16) );
            }
        }
    }

    // ...then add the sprites
    TBltAddXlatDstTrans         sprite_blitter_f( 0, 0, sprite_xlat_table_ );
    TBltAddXlatDstTransReverse  sprite_blitter_r( 0, 0, sprite_xlat_table_ );

    for( int i=0; i<24; i++ ) {
        unsigned char * sram = main_board_->ram_ + 0x9010 + i*2;

        int x = sram[0x401] - 16;
        int y = sram[0x000];

        if( (y < 16) || (y > 240) )
            continue;

        unsigned mode = ((sram[0x400] & 0x40) ? opFlipY : 0) | ((sram[0x400] & 0x80) ? 0 : opFlipX);
        TBltAddXlatDstTrans * blitter = (mode & opFlipX) ? &sprite_blitter_r : &sprite_blitter_f;

        screen()->bits()->copy( x, y, sprite_data_, 0, (sram[0x001] & 0x3F)*16, 16, 16, mode, blitter->color((sram[0x400] & 0x0F)*16) );
    }

    return screen();
}

// Update the internal tables after a ROM has changed
void Pooyan::onVideoROMsChanged()
{
    int i;

    // Refresh palette
    for( i=0; i<32; i++ ) {
        palette()->setColor( i, TPalette::decodeByte(palette_prom_[i]) );
    }

    // Refresh character and sprite color tables
    for( i=0; i<256; i++ ) {
        sprite_xlat_table_[i] = sprite_table_prom_[i] & 0x0F;      // Colors from 0x00 to 0x0F
        char_xlat_table_[i] = (char_table_prom_[i] & 0x0F)+ 0x10;  // Colors from 0x10 to 0x1F
    }

    // Decode character set
    for( i=0; i<256; i++ ) {
        decodeChar( video_rom_ + 16*i, &char_data_, 0, i*8 );
    }

    // Decode sprite set
    for( i=0; i<64; i++ ) {
        decodeChar( video_rom_ + 0x2000 + 64*i,      &sprite_data_, 0, 16*i+8 );
        decodeChar( video_rom_ + 0x2000 + 64*i + 16, &sprite_data_, 0, 16*i+0 );
        decodeChar( video_rom_ + 0x2000 + 64*i + 32, &sprite_data_, 8, 16*i+8 );
        decodeChar( video_rom_ + 0x2000 + 64*i + 48, &sprite_data_, 8, 16*i+0 );
    }
}

PooyanMainBoard::PooyanMainBoard( PooyanSoundBoard * sound_board )
{
    // Initialize the CPU and the RAM
    cpu_ = new Z80( *this );
    memset( ram_, 0xFF, sizeof(ram_) );

    // Initialize the board variables
    port0_ = 0xFF;
    port1_ = 0xFF;
    port2_ = 0xFF;
    coin_counter_1_ = 0;
    coin_counter_2_ = 0;
    dip_switches_1_ = 0x02;
    dip_switches_2_ = 0xFF;
    sound_board_ = sound_board;

    // Reset the machine
    reset();
}

void PooyanMainBoard::reset()
{
    cpu_->reset();
    memset( ram_+32*1024, 0xFF, 6*1024 );
    output_devices_ = 0;
}

void PooyanMainBoard::run()
{
    setOutputFlipFlop( HaveSoundInterrupt, 0 );

    cpu_->run( CpuCyclesPerFrame );

    // If interrupts are enabled, force a CPU interrupt with the vector
    // set by the program
    if( output_devices_ & InterruptEnabled ) {
        cpu_->nmi();
    }
}

void PooyanMainBoard::setOutputFlipFlop( unsigned char bit, unsigned char value )
{
    if( value ) {
        output_devices_ |= bit;
    }
    else {
        output_devices_ &= ~bit;
    }
}

unsigned char PooyanMainBoard::readByte( unsigned addr ) 
{
    addr &= 0xFFFF;

    if( addr < sizeof(ram_) )
        return ram_[addr];

    // Address is not in RAM, check to see if it's a memory mapped register
    switch( addr  ) {
    case 0xA000:
        return dip_switches_1_; // DSW1
    case 0xA080: 
        return port0_; // IN0
    case 0xA0A0: 
        return port1_; // IN1
    case 0xA0C0:
        return port2_; // IN2
    case 0xA0E0:  
        return dip_switches_2_; // DSW2
        break;
    }

    return 0xFF;
}

void PooyanMainBoard::writeByte( unsigned addr, unsigned char b )
{
    addr &= 0xFFFF;

    if( addr < 0x8000 ) {
        // This is a ROM address, do not write into it!
    }
    else if( addr < 0x8400 ) { // Color memory
        ram_[addr] = b;
    }
    else if( addr < 0x8800 ) { // Video memory
        ram_[addr] = b;
    }
    else if( addr < 0x8FFF ) { // Standard memory
        ram_[addr] = b;
    }
    else if( addr < 0x97FF ) { // Sprites
        ram_[addr] = b;
    }
    else { 
        // Memory mapped ports
        switch( addr ) {
        case 0xA000:
            // Watchdog?
            break;
        case 0xA028:
            // ??? Seems to always write a zero at rare but specific moments...
            break;
        case 0xA100:
            // Audio CPU command
            sound_board_->soundChip(0)->setRegister( AY_3_8910::PortA, b );
            break;
        case 0xA180:
            // Interrupt enable
            setOutputFlipFlop( InterruptEnabled, b & 0x01 );
            break;
        case 0xA181:
            // Interrupt trigger on audio CPU
            if( (output_devices_ & SoundInterrupt) && (b == 0) ) {
                setOutputFlipFlop( HaveSoundInterrupt, 1 );
            }
            setOutputFlipFlop( SoundInterrupt, b & 0x01 );
            break;
        case 0xA183:
            // Increment coin meter 1
            if( (output_devices_ & CoinMeter1) && (b == 0) ) {
                coin_counter_1_++;
            }
            setOutputFlipFlop( CoinMeter1, b & 0x01 );
            break;
        case 0xA184:
            // Increment coin meter 2
            if( (output_devices_ & CoinMeter2) && (b == 0) ) {
                coin_counter_2_++;
            }
            setOutputFlipFlop( CoinMeter2, b & 0x01 );
            break;
        case 0xA187:
            // Flip screen
            setOutputFlipFlop( FlipScreen, b & 0x01 );
            break;
        }
    }
}

PooyanSoundBoard::PooyanSoundBoard() : Z80_AY3_SoundBoard( 2, 14318180/8, 8*1024, 1*1024, 0x3000 )
{
    timer_clock_ = 0;
}

void PooyanSoundBoard::run()
{
    // Update the timer clock and reset the number of cycles (to avoid overflow)
    timer_clock_ = (timer_clock_ + cpu()->getCycles()) % 5120;

    cpu()->setCycles( 0 );

    Z80_AY3_SoundBoard::run();
}

/*
    According to the MAME driver (timeplt.c) the clock that is connected
    to port B of the first AY-3-8910 uses the same (audio) CPU clock
    with a 5120 divider that is formed by a standard 512 divider followed
    by a LS90 chip that further divides by 10 using a bi-quinary sequence.
    I cannot find the original schematics though, so I'm now using
    a standard sequence just to see if someone notices the difference! :-)
*/
static unsigned char DM74LS90_Clock[10] =
{
    0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90
	// Alternate (bi-quinary) count sequence:
    // 0x00, 0x10, 0x20, 0x30, 0x40, 0x90, 0xa0, 0xb0, 0xa0, 0xd0
};

unsigned char PooyanSoundBoard::onReadByte( unsigned addr )
{
    if( addr == 0x4000 ) {
        // AY-3-8910/1 read port. Since I/O port B is connected to the LS90 clock chip
        // we must update it now, before someone has a chance to read it
        unsigned clock = (timer_clock_ + cpu()->getCycles()) % 5120;
        soundChip(0)->setRegister( AY_3_8910::PortB, DM74LS90_Clock[clock/512] );

        return soundChip(0)->readData();
    }
    else if( addr == 0x6000 ) // AY-3-8910/2 read port
        return soundChip(1)->readData();

    return 0xFF;
}

void PooyanSoundBoard::onWriteByte( unsigned addr, unsigned char value )
{
    if( addr == 0x4000 )        // AY-3-8910/1 write port
        soundChip(0)->writeData( value );
    else if( addr == 0x5000 ) // AY-3-8910/1 control port
        soundChip(0)->writeAddress( value );
    else if( addr == 0x6000 ) // AY-3-8910/2 write port
        soundChip(1)->writeData( value );
    else if( addr == 0x7000 ) // AY-3-8910/2 control port
        soundChip(1)->writeAddress( value );
    
   // Port mapped at 0x8000 controls the sound RC output filter, but it's not emulated
}
