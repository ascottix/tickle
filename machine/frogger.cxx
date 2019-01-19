/*
    Frogger arcade machine emulator

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "frogger.h"

// Output flip-flops
enum {
    FlipScreenX         = 0x01,
    FlipScreenY         = 0x02,
    InterruptEnabled    = 0x08,
    SoundInterrupt      = 0x10,
    HaveSoundInterrupt  = 0x20,
    CoinMeter1          = 0x40,
    CoinMeter2          = 0x80,
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
    CpuCyclesAfterInterrupt = 2500, // Between main CPU NMI and VBLANK
    SoundCpuCyclesPerFrame  = SoundCpuClock / VideoFrequency
};

enum { 
    Ef26, Ef27, Ef37, Ef608, Ef609, Ef610, Ef6L, Ef606, Ef607, 
    OptLives, OptCabinet, OptCoin, OptCoin2
};

// Machine info
static TMachineInfo FroggerInfo = { 
    "frogger", "Frogger", S_Konami, 1981, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};
 
static TGameRegistrationHandler reg1( &FroggerInfo, Frogger::createInstance );

bool Frogger::initialize( TMachineDriverInfo * info )
{
    // Declare resources
    resourceHandler()->add( Ef26, "frogger.26", 0x1000, efROM, main_board_->rom_+0x0000 );
    resourceHandler()->add( Ef27, "frogger.27", 0x1000, efROM, main_board_->rom_+0x1000 );
    resourceHandler()->add( Ef37, "frsm3.7",    0x1000, efROM, main_board_->rom_+0x2000 );

    resourceHandler()->add( Ef608, "frogger.608", 0x800, efSoundPROM, sound_board_.rom()+0x0000 );
    resourceHandler()->add( Ef609, "frogger.609", 0x800, efSoundPROM, sound_board_.rom()+0x0800 );
    resourceHandler()->add( Ef610, "frogger.610", 0x800, efSoundPROM, sound_board_.rom()+0x1000 );

    resourceHandler()->add( Ef607, "frogger.607", 0x800, efVideoROM, video_rom_+0x0000 );
    resourceHandler()->add( Ef606, "frogger.606", 0x800, efVideoROM, video_rom_+0x0800 );

    resourceHandler()->add( Ef6L, "pr-91.6l", 0x20, efPalettePROM, palette_prom_ );

    resourceHandler()->assignToMachineDriverInfo( info );

    // Setup user interface
    TUserInterface * ui = info->userInterface();

    TUiOption * option;
    TUiOptionGroup * group = ui->addGroup( S_DSW_1, dtDipSwitch );

    option = group->add( OptLives, "Frogs per game", 0, 0x03 );
    option->add( "3", 0x00 );
    option->add( "5", 0x01 );
    option->add( "7", 0x02 );
    option->add( S_FreePlay, 0x03 );

    option = group->add( OptCoin, S_Coin, 3, 0x06 );
    option->add( S_1Coin1Play, 0x00 );
    option->add( S_2Coin1Play, 0x02 );
    option->add( "Slot 1: 1C/1P, slot 2: 1C/3P", 0x04 );
    option->add( "Slot 1: 1C/1P, slot 2: 1C/6P", 0x06 );

    option = group->add( OptCabinet, S_Cabinet, 0, 0x08 );
    option->add( S_Upright, 0x00 );
    option->add( S_Cocktail, 0x08 );

    optionHandler()->add( &main_board_->port1_, ui, OptLives );
    optionHandler()->add( &main_board_->port2_, ui, OptCoin, OptCabinet );

    // Setup palette (non-PROM colors)
    TPalette * palette = screen()->palette();

    palette->setColor( BackgroundPaletteIndex, TPalette::encodeColor(0x00,0x00,0x55) ); // Blue

    return true;
}

bool Frogger::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    unsigned char rombuf[0x800];

    // Need to refresh some internal tables if the video/color ROMs change
    refresh_roms_ |= (id >= Ef6L);

    if( (id == Ef608) || (id == Ef606) ) {
        // The first sound ROM and the second graphics ROM have lines 0 and 1 swapped
        assert( len = sizeof(rombuf) );

        for( unsigned i=0; i<len; i ++ ) {
            rombuf[i] = (buf[i] & 0xFC) | ((buf[i] & 0x01) << 1) | ((buf[i] & 0x02) >> 1);
        }

        buf = rombuf;
    }

    return 0 == resourceHandler()->handle( id, buf, len );
}

Frogger::Frogger( FroggerMainBoard * board ) :
    char_data_( 8, 8*256 ),     // 256 8x8 characters
    sprite_data_( 16, 16*64 )   // 64 16x16 sprites
{
    main_board_ = board;

    refresh_roms_ = false;

    createScreen( ScreenWidth, ScreenHeight, ScreenColors );

    eventHandler()->add( idCoinSlot1,       ptInverted, &main_board_->port0_, 0x80 );
    eventHandler()->add( idCoinSlot2,       ptInverted, &main_board_->port0_, 0x40 );
    eventHandler()->add( idKeyService1,     ptInverted, &main_board_->port0_, 0x04 );
    eventHandler()->add( idKeyStartPlayer1, ptInverted, &main_board_->port1_, 0x80 );
    eventHandler()->add( idKeyStartPlayer2, ptInverted, &main_board_->port1_, 0x40 );

    setJoystickHandler( 0, new TJoystickToPortHandler(idJoyP1Joystick1, ptInverted, jm4Way) );
    joystickHandler(0)->setPort( jpLeft,  &main_board_->port0_, 0x20 );
    joystickHandler(0)->setPort( jpRight, &main_board_->port0_, 0x10 );
    joystickHandler(0)->setPort( jpUp,    &main_board_->port2_, 0x10 );
    joystickHandler(0)->setPort( jpDown,  &main_board_->port2_, 0x40 );

    setJoystickHandler( 1, new TJoystickToPortHandler(idJoyP2Joystick1, ptInverted, jm4Way) );
    joystickHandler(1)->setPort( jpLeft,  &main_board_->port1_, 0x20 );
    joystickHandler(1)->setPort( jpRight, &main_board_->port1_, 0x10 );
    joystickHandler(1)->setPort( jpUp,    &main_board_->port0_, 0x01 );
    joystickHandler(1)->setPort( jpDown,  &main_board_->port2_, 0x01 );

    registerDriver( FroggerInfo );
}

FroggerSoundBoard::FroggerSoundBoard() : Z80_AY3_SoundBoard( 1, 14318180/8, 8*1024, 1*1024, 0x4000 )
{
    timer_clock_ = 0;
}

void FroggerSoundBoard::run()
{
    // Update the timer clock and reset the number of cycles (to avoid overflow)
    timer_clock_ = (timer_clock_ + cpu()->getCycles()) % 5120;

    cpu()->setCycles( 0 );

    Z80_AY3_SoundBoard::run();
}

static unsigned char DM74LS90_Clock[10] =
{
	0x00, 0x10, 0x08, 0x18, 0x40, 0x90, 0x88, 0x98, 0x88, 0xD0
};

unsigned char FroggerSoundBoard::readPort( unsigned addr )
{
    switch( addr & 0xFF ) {
    case 0x40:
        {
            // AY-3-8910/1 read port. Since I/O port B is connected to the LS90 clock chip
            // we must update it now, before someone has a chance to read it
            unsigned clock = (timer_clock_ + cpu()->getCycles()) % 5120;
            soundChip(0)->setRegister( AY_3_8910::PortB, DM74LS90_Clock[clock/512] );
        }

        return soundChip(0)->readData();
    }

    return 0;
}

void FroggerSoundBoard::writePort( unsigned addr, unsigned char value )
{
    switch( addr & 0xFF ) {
    case 0x40: // AY-3-8910/2 write port
        soundChip(0)->writeData( value );
        break;
    case 0x80: // AY-3-8910/2 control port
        soundChip(0)->writeAddress( value );
        break;
    }
}

FroggerMainBoard::FroggerMainBoard()
{
    // Initialize the CPU and the RAM
    cpu_ = new Z80( *this );

    memset( video_ram_, 0, sizeof(video_ram_) );

    // Initialize the board variables
    port0_ = 0xFF;
    port1_ = 0xFC;
    port2_ = 0xF1;

    coin_counter_1_ = 0;
    coin_counter_2_ = 0;

    sound_command_ = 0;

    // Reset the machine
    reset();
}

void FroggerMainBoard::reset()
{
    cpu_->reset();

    output_devices_ = 0;
}

void FroggerMainBoard::run()
{
    setOutputFlipFlop( HaveSoundInterrupt, 0 );

    cpu_->run( CpuCyclesPerFrame - CpuCyclesAfterInterrupt );

    // If interrupts are enabled, force a CPU interrupt with the vector
    // set by the program
    if( output_devices_ & InterruptEnabled ) {
        cpu_->nmi();
    }

    cpu_->run( CpuCyclesAfterInterrupt );

}

void FroggerMainBoard::setOutputFlipFlop( unsigned char bit, unsigned char value )
{
    if( value ) {
        output_devices_ |= bit;
    }
    else {
        output_devices_ &= ~bit;
    }
}

unsigned char FroggerMainBoard::readByte( unsigned addr ) 
{
    addr &= 0xFFFF;

    if( addr < sizeof(rom_) ) {
        return rom_[addr];
    }
    else if( addr >= 0x8000 ) {
        if( addr <= 0x87FF ) {
            return ram_[ addr - 0x8000 ];
        }
        
        if( addr == 0x8800 ) {
            // Reset watchdog timer
        }
        else if( (addr >= 0xA800) && (addr <= 0xABFF) ) {
            // Video RAM
            return video_ram_[ addr - 0xA800 ];
        }
        else if( (addr >= 0xB000) && (addr <= 0xB0FF) ) {
            // Special memory
            return special_ram_[ addr - 0xB000 ];
        }
        else switch( addr ) {
            case 0xE000:
            case 0xE001:
                return port0_;
            case 0xE002:
            case 0xE003:
                return port1_;
            case 0xE004:
            case 0xE005:
                return port2_;
        }
    }

    return 0xFF;
}

void FroggerMainBoard::writeByte( unsigned addr, unsigned char b )
{
    addr &= 0xFFFF;

    if( addr >= 0x8000 ) {
        if( addr <= 0x87FF ) {
            ram_[addr - 0x8000] = b;
        }
        else if( (addr >= 0xA800) && (addr <= 0xABFF) ) {
            video_ram_[ addr - 0xA800 ] = b;
        }
        else if( (addr >= 0xB000) && (addr <= 0xB0FF) ) {
            special_ram_[ addr - 0xB000 ] = b;
        }
        else { 
            // Memory mapped ports
            switch( addr ) {
            case 0xB808:
                // Interrupt enable
                setOutputFlipFlop( InterruptEnabled, b & 0x01 );
                break;
            case 0xB80C:
                // Flip screen vertically
                setOutputFlipFlop( FlipScreenY, b & 0x01 );
                break;
            case 0xB810:
                // Flip screen horizontally
                setOutputFlipFlop( FlipScreenX, b & 0x01 );
                break;
            case 0xB818:
                // Increment coin meter
                if( (output_devices_ & CoinMeter1) && (b == 0) ) {
                    coin_counter_1_++;
                }
                setOutputFlipFlop( CoinMeter1, b & 0x01 );
                break;
            case 0xB81C:
                // Increment coin meter
                if( (output_devices_ & CoinMeter2) && (b == 0) ) {
                    coin_counter_2_++;
                }
                setOutputFlipFlop( CoinMeter2, b & 0x01 );
                break;
            case 0xD000:
            case 0xD001:
                // 8255 chip #1 port A/AY-3-8910 port A (sound command)
                sound_command_ = b;
                break;
            case 0xD002:
            case 0xD003:
                // 8255 chip #1 port A/AY-3-8910 port B (interrupt trigger on audio CPU)
                if( (output_devices_ & SoundInterrupt) && (b == 0) ) {
                    setOutputFlipFlop( HaveSoundInterrupt, 1 );
                }
                setOutputFlipFlop( SoundInterrupt, b & 0x08 );
                break;
            }
        }
    }
}

void Frogger::reset()
{
    main_board_->reset();

    sound_board_.reset();
}

void Frogger::run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate )
{
    if( refresh_roms_ ) {
        onVideoROMsChanged();
        refresh_roms_ = false;
    }

    // Run the main CPU
    main_board_->run();

    // Run the sound CPU
    sound_board_.soundChip(0)->setRegister( AY_3_8910::PortA, main_board_->sound_command_ );
    
    if( main_board_->output_devices_ & HaveSoundInterrupt ) {
        sound_board_.interrupt( 0x00 );
    }

    sound_board_.run();

    sound_board_.playSound( SoundCpuCyclesPerFrame, frame->getMixer(), samplesPerFrame, samplingRate );

    // Render the video
    frame->setVideo( renderVideo() );
}

void Frogger::decodeChar( const unsigned char * src, TBitBlock * bb, int ox, int oy, int planes, unsigned plane_size )
{
    for( int y=0; y<8; y++ ) {
        for( int x=0; x<8; x++ ) {
            unsigned char mask = 1 << x;
            unsigned char color = 0;
            unsigned plane_offset = 0;

            for( int plane=planes-1; plane>=0; plane-- ) {
                if( src[plane_offset] & mask ) {
                    color |= (1 << plane);
                }

                plane_offset += plane_size;
            }

            bb->setPixel( ox+7-y, oy+7-x, color ); // Note that x and y are inverted!
        }

        src++;
    }
}

void Frogger::decodeSprite( const unsigned char * src, TBitBlock * bb, int ox, int oy )
{
    decodeChar( src,     bb, ox + 8, oy );
    decodeChar( src + 8, bb, ox + 8, oy + 8 );
    decodeChar( src +16, bb, ox + 0, oy );
    decodeChar( src +24, bb, ox + 0, oy + 8 );
}

TBitmapIndexed * Frogger::renderVideo()
{
    unsigned char * video = main_board_->video_ram_;
    unsigned char * attr = main_board_->special_ram_;

    TBltAddSrcZeroTrans         blitter_f(0);
    TBltAddSrcZeroTransReverse  blitter_r(0);

    unsigned mode = opAdd |
        (main_board_->output_devices_ & FlipScreenX ? opFlipX : 0) | 
        (main_board_->output_devices_ & FlipScreenY ? opFlipY : 0);

    TBltAddSrcZeroTrans * blitter = (mode & opFlipX) ? &blitter_r : &blitter_f;

    // Draw the background
    screen()->bits()->fill( 0, 0, ScreenWidth, ScreenHeight / 2, BackgroundPaletteIndex );
    screen()->bits()->fill( 0, ScreenHeight / 2, ScreenWidth, ScreenHeight / 2, 0 );

    // ...then draw the tiles...
    for( int y=0; y<ScreenHeightChars; y++ ) {
        unsigned scroll = attr[ y*2 ];
        unsigned color = attr[ y*2 + 1 ];

        color = ((color >> 1) & 0x03) | ((color << 2) & 0x04);

        scroll = (scroll >> 4) | ((scroll & 0x0F) << 4);

        for( int x=0; x<32; x++ ) {
            unsigned offset = 32*(31-x) + y;

            int cx = ((x*8 + scroll) & 0xFF) - 16;
            int cy = y*8;

            if( cx >= -7 && cx < 224 ) {
                if( mode & opFlipX ) cx = 216 - cx;
                if( mode & opFlipY ) cy = 247 - cy;
                screen()->bits()->copy( cx, cy, char_data_, 0, video[offset]*8, 8, 8, mode, blitter->color(color*4) );
            }
        }
    }

    // ...and finally the sprites
    for( int i=7; i>=0; i-- ) {
        unsigned char * sram = main_board_->special_ram_ + 0x40 + i*4;

        int y = sram[3] + 1;
        int x = (sram[0] >> 4) | ((sram[0] & 0x0F) << 4);

        x -= 16;

        unsigned s_mode = mode ^ (((sram[1] & 0x40) ? opFlipY : 0) | ((sram[1] & 0x80) ? opFlipX : 0));

        if( mode & opFlipX ) x = 208-x;
        if( mode & opFlipY ) y = 239-y;

        blitter = (s_mode & opFlipX) ? &blitter_r : &blitter_f;

        unsigned color = sram[2];

        color = ((color >> 1) & 0x03) | ((color << 2) & 0x04);

        screen()->bits()->copy( x, y, sprite_data_, 0, (sram[1] & 0x3F)*16, 16, 16, s_mode, blitter->color(color*4) );
    }

    return screen();
}

// Update the internal tables after a ROM has changed
void Frogger::onVideoROMsChanged()
{
    int i;

    // Refresh palette
    for( i=0; i<32; i++ ) {
        palette()->setColor( i, TPalette::decodeByte(palette_prom_[i]) );
    }

    // Decode character set
    for( i=0; i<256; i++ ) {
        decodeChar( video_rom_ + 8*i, &char_data_, 0, i*8 );
    }

    // Decode sprite set
    for( i=0; i<64; i++ ) {
        decodeSprite( video_rom_ + 32*i, &sprite_data_, 0, 16*i );
    }
}
