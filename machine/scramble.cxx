/*
    Scramble arcade machine emulator

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "scramble.h"

// Output flip-flops
enum {
    FlipScreenX         = 0x01,
    FlipScreenY         = 0x02,
    CoinMeter           = 0x04,
    InterruptEnabled    = 0x08,
    SoundInterrupt      = 0x10,
    HaveSoundInterrupt  = 0x20,
    BackgroundEnabled   = 0x40,
    StarsEnabled        = 0x80,
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
    MicrosecondsPerFrame    = 1000000 / VideoFrequency,
    CpuCyclesPerFrame       = CpuClock / VideoFrequency,
    CpuCyclesAfterInterrupt = 2000, // Between main CPU NMI and VBLANK
    SoundCpuCyclesPerFrame  = SoundCpuClock / VideoFrequency
};

enum { 
    Ef2D, Ef2E, Ef2F, Ef2H, Ef2J, Ef2L, Ef2M, Ef2P, Ef5C, Ef5D, Ef5E, Ef5F, Ef5H, Ef6E,
    OptLives, OptCabinet, OptCoin, OptCoin2
};

// Machine info
static TMachineInfo ScrambleInfo = { 
    "scramble", "Scramble", S_Konami, 1981, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};
 
static TMachineInfo AmidarOnScrambleInfo = { 
    "amidars", "Amidar (on Scramble hardware)", S_Konami, 1982, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo AtlantisInfo = { 
    "atlantis", "Battle of Atlantis", S_Comsoft, 1981, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo TheEndInfo = { 
    "theend", "The End", S_Konami, 1980, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo MarsInfo = { 
    "mars", "Mars", S_Artic, 1981, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TGameRegistrationHandler reg1( &ScrambleInfo, Scramble::createInstance );
static TGameRegistrationHandler reg2( &AmidarOnScrambleInfo, AmidarOnScramble::createInstance );
static TGameRegistrationHandler reg3( &AtlantisInfo, Atlantis::createInstance );
static TGameRegistrationHandler reg4( &TheEndInfo, TheEnd::createInstance );
static TGameRegistrationHandler reg5( &MarsInfo, Mars::createInstance );

bool Scramble::initialize( TMachineDriverInfo * info )
{
    // Declare resources
    resourceHandler()->add( Ef2D,      "2d.k",   0x800, efROM, main_board_->ram_+0x0000 );
    resourceHandler()->add( Ef2E,      "2e.k",   0x800, efROM, main_board_->ram_+0x0800 );
    resourceHandler()->add( Ef2F,      "2f.k",   0x800, efROM, main_board_->ram_+0x1000 );
    resourceHandler()->add( Ef2H,      "2h.k",   0x800, efROM, main_board_->ram_+0x1800 );
    resourceHandler()->add( Ef2J,      "2j.k",   0x800, efROM, main_board_->ram_+0x2000 );
    resourceHandler()->add( Ef2L,      "2l.k",   0x800, efROM, main_board_->ram_+0x2800 );
    resourceHandler()->add( Ef2M,      "2m.k",   0x800, efROM, main_board_->ram_+0x3000 );
    resourceHandler()->add( Ef2P,      "2p.k",   0x800, efROM, main_board_->ram_+0x3800 );

    resourceHandler()->add( Ef5C,        "5c",   0x800, efSoundPROM, sound_board_.rom()+0x0000 );
    resourceHandler()->add( Ef5D,        "5d",   0x800, efSoundPROM, sound_board_.rom()+0x0800 );
    resourceHandler()->add( Ef5E,        "5e",   0x800, efSoundPROM, sound_board_.rom()+0x1000 );

    resourceHandler()->add( Ef5F,      "5f.k",   0x800, efVideoROM, video_rom_+0x0000 );
    resourceHandler()->add( Ef5H,      "5h.k",   0x800, efVideoROM, video_rom_+0x0800 );

    resourceHandler()->add( Ef6E, "82s123.6e",   0x20, efPalettePROM, palette_prom_ );

    resourceHandler()->assignToMachineDriverInfo( info );

    // Setup user interface
    TUserInterface * ui = info->userInterface();

    TUiOption * option;
    TUiOptionGroup * group = ui->addGroup( S_DSW_1, dtDipSwitch );

    option = group->add( OptLives, "Rockets per game", 0, 0x03 );
    option->add( "3", 0x00 );
    option->add( "4", 0x01 );
    option->add( "5", 0x02 );
    option->add( S_FreePlay, 0x03 );

    option = group->add( OptCoin, S_Coin, 0, 0x06 );
    option->add( "Slot 1: 1C/1P, slot 2: 2C/1P", 0x00 );
    option->add( "Slot 1: 1C/2P, slot 2: 1C/1P", 0x02 );
    option->add( "Slot 1: 1C/3P, slot 2: 3C/1P", 0x04 );
    option->add( "Slot 1: 1C/4P, slot 2: 4C/1P", 0x06 );

    option = group->add( OptCabinet, S_Cabinet, 0, 0x08 );
    option->add( S_Upright, 0x00 );
    option->add( S_Cocktail, 0x08 );

    optionHandler()->add( &main_board_->port1_, ui, OptLives );
    optionHandler()->add( &main_board_->port2_, ui, OptCoin, OptCabinet );

    return true;
}

bool Scramble::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    // Need to refresh some internal tables if the video/color ROMs change
    refresh_roms_ |= (id >= Ef5F);

    return 0 == resourceHandler()->handle( id, buf, len );
}

Scramble::Scramble( ScrambleMainBoard * board ) :
    char_data_( 8, 8*256 ),     // 256 8x8 characters
    sprite_data_( 16, 16*64 )   // 64 16x16 sprites
{
    main_board_ = board;
    refresh_roms_ = false;

    createScreen( ScreenWidth, ScreenHeight, ScreenColors );

    eventHandler()->add( idCoinSlot1,        ptInverted, &main_board_->port0_, 0x80 );
    eventHandler()->add( idCoinSlot2,        ptInverted, &main_board_->port0_, 0x40 );
    eventHandler()->add( idKeyService1,      ptInverted, &main_board_->port0_, 0x04 );
    eventHandler()->add( idKeyP1Action1,     ptInverted, &main_board_->port0_, 0x08 );
    eventHandler()->add( idKeyP1Action2,     ptInverted, &main_board_->port0_, 0x02 );
    eventHandler()->add( idKeyP2Action1,     ptInverted, &main_board_->port1_, 0x08 );
    eventHandler()->add( idKeyP2Action2,     ptInverted, &main_board_->port1_, 0x04 );
    eventHandler()->add( idKeyStartPlayer1,  ptInverted, &main_board_->port1_, 0x80 );
    eventHandler()->add( idKeyStartPlayer2,  ptInverted, &main_board_->port1_, 0x40 );

    setJoystickHandler( 0, new TJoystickToPortHandler(idJoyP1Joystick1, ptInverted, jm8Way) );
    joystickHandler(0)->setPort( jpLeft,  &main_board_->port0_, 0x20 );
    joystickHandler(0)->setPort( jpRight, &main_board_->port0_, 0x10 );
    joystickHandler(0)->setPort( jpUp,    &main_board_->port2_, 0x10 );
    joystickHandler(0)->setPort( jpDown,  &main_board_->port2_, 0x40 );

    setJoystickHandler( 1, new TJoystickToPortHandler(idJoyP2Joystick1, ptInverted, jm8Way) );
    joystickHandler(1)->setPort( jpLeft,  &main_board_->port1_, 0x20 );
    joystickHandler(1)->setPort( jpRight, &main_board_->port1_, 0x10 );
    joystickHandler(1)->setPort( jpUp,    &main_board_->port0_, 0x01 );
    joystickHandler(1)->setPort( jpDown,  &main_board_->port2_, 0x01 );

    registerDriver( ScrambleInfo );

    // Setup palette (non-PROM colors)
    TPalette * palette = screen()->palette();

    palette->setColor( BackgroundPaletteIndex, TPalette::encodeColor(0x00,0x00,0x55) ); // Blue
    palette->setColor( BulletPaletteIndex, TPalette::encodeColor(0xFF,0xFF,0x00) );     // Yellow

    unsigned char starfieldColors[4] = { 0x00, 0x66, 0x99, 0xFF };

    for( int i=0; i<64; i++ ) {
        palette->setColor( i + StarsPaletteIndex, TPalette::encodeColor(
            starfieldColors[ (i >> 0) & 0x03 ],
            starfieldColors[ (i >> 2) & 0x03 ],
            starfieldColors[ (i >> 4) & 0x03 ] ) );
    }

    // Generate starfield (can't get info about this chip, got the setup
    // and the blinking algorithm from the MAME driver in galaxian.c)
    int stars = 0;
    unsigned generator = 0;

	for( int x=0; x<256; x++ ) {
		for( int y=0; y<512; y++ ) {
            unsigned bit = ((~generator >> 16) & 0x01) ^ ((generator >> 4) & 0x01);

			generator = (generator << 1) | bit;

			if( ((~generator >> 16) & 0x01) && (generator & 0xFF) == 0xFF ) {
				int color = (~(generator >> 8)) & 0x3F;

				if( color ) {
                    assert( stars < NumOfStars );

					starfield_[stars].x = x;
					starfield_[stars].y = y;
					starfield_[stars].color = color;

					stars++;
				}
			}
		}
	}

    // Initialize starfield counters: the blink period is calculated using the
    // formula for the period of a 555 timer (astable configuration) with the
    // values from the schematics (RA=100Kohm, RB=10Kohm, C=10uF)
    starfield_blink_state_ = 0;
    starfield_blink_timer_ = 0;
    starfield_blink_period_ = (unsigned)(0.693 * (100000.0 + 2*10000.0) * 0.00001 * 1000000);
    starfield_scroll_pos_ = 0;
}

ScrambleSoundBoard::ScrambleSoundBoard() : Z80_AY3_SoundBoard( 2, 14318180/8, 8*1024, 4*1024, 0x8000 )
{
    timer_clock_ = 0;
}

void ScrambleSoundBoard::run()
{
    // Update the timer clock and reset the number of cycles (to avoid overflow)
    timer_clock_ = (timer_clock_ + cpu()->getCycles()) % 5120;

    cpu()->setCycles( 0 );

    Z80_AY3_SoundBoard::run();
}

static unsigned char DM74LS90_Clock[10] =
{
    0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90
	// Alternate (bi-quinary) count sequence:
    // 0x00, 0x10, 0x20, 0x30, 0x40, 0x90, 0xa0, 0xb0, 0xa0, 0xd0
};

unsigned char ScrambleSoundBoard::readPort( unsigned addr )
{
    switch( addr & 0xFF ) {
    case 0x20:
        return soundChip(0)->readData();
    case 0x80:
        {
            // AY-3-8910/1 read port. Since I/O port B is connected to the LS90 clock chip
            // we must update it now, before someone has a chance to read it
            unsigned clock = (timer_clock_ + cpu()->getCycles()) % 5120;
            soundChip(1)->setRegister( AY_3_8910::PortB, DM74LS90_Clock[clock/512] );
        }

        return soundChip(1)->readData();
    }

    return 0;
}

void ScrambleSoundBoard::writePort( unsigned addr, unsigned char value )
{
    switch( addr & 0xFF ) {
    case 0x10: // AY-3-8910/1 control port
        soundChip(0)->writeAddress( value );
        break;
    case 0x20: // AY-3-8910/1 write port
        soundChip(0)->writeData( value );
        break;
    case 0x40: // AY-3-8910/2 control port
        soundChip(1)->writeAddress( value );
        break;
    case 0x80: // AY-3-8910/2 write port
        soundChip(1)->writeData( value );
        break;
    }
}

ScrambleMainBoard::ScrambleMainBoard()
{
    // Initialize the CPU and the RAM
    cpu_ = new Z80( *this );
    memset( ram_, 0xFF, sizeof(ram_) );

    // Initialize the board variables
    port0_ = 0xFF;
    port1_ = 0xFF;
    port2_ = 0xFF;

    coin_counter_ = 0;

    sound_command_ = 0;

    // Reset the machine
    reset();
}

void ScrambleMainBoard::reset()
{
    cpu_->reset();
    output_devices_ = 0;
}

void ScrambleMainBoard::run()
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

void ScrambleMainBoard::setOutputFlipFlop( unsigned char bit, unsigned char value )
{
    if( value ) {
        output_devices_ |= bit;
    }
    else {
        output_devices_ &= ~bit;
    }
}

unsigned char ScrambleMainBoard::readByte( unsigned addr ) 
{
    addr &= 0xFFFF;

    if( addr < sizeof(ram_) )
        return ram_[addr];

    // Address is not in RAM, check to see if it's a memory mapped register
    switch( addr  ) {
    case 0x7000:
    case 0x7800:
        // Reset watchdog
        break;
    case 0x8100:
        // Read 8255 chip #1: port A
        return port0_;
    case 0x8101:
        // Read 8255 chip #1: port B
        return port1_;
    case 0x8102:
        // Read 8255 chip #1: port C
        return port2_;
    case 0x8202:
        // Read 8255 chip #2: port C
        {
            // Note: this is only compatible with the limited data available to me!
            unsigned char a = cpu_->PC & 0x0F;
            unsigned char r = 0;

            if( (!(a & 0x01) &&  (a & 0x08) ) ||  (a & 0x04) ) r |= 0x20;
            if(   (a & 0x01) || !(a & 0x02)                  ) r |= 0x40;
            if( (!(a & 0x01) &&  (a & 0x08) ) || !(a & 0x02) ) r |= 0x90;

            return r;
        }
    }

    return 0xFF;
}

void ScrambleMainBoard::writeByte( unsigned addr, unsigned char b )
{
    addr &= 0xFFFF;

    if( addr < 0x4000 ) {
        // This is a ROM address, do not write into it!
    }
    else if( addr < sizeof(ram_) ) {
        ram_[addr] = b;
        if( addr >= 0x4C00 && addr <= 0x4FFF ) {
            ram_[addr-0x400] = b;
        }
    }
    else { 
        // Memory mapped ports
        switch( addr ) {
        case 0x6801:
            // Interrupt enable
            setOutputFlipFlop( InterruptEnabled, b & 0x01 );
            break;
        case 0x6802:
            // Increment coin meter
            if( (output_devices_ & CoinMeter) && (b == 0) ) {
                coin_counter_++;
            }
            setOutputFlipFlop( CoinMeter, b & 0x01 );
            break;
        case 0x6803:
            // Enable background (blue)
            setOutputFlipFlop( BackgroundEnabled, b & 0x01 );
            break;
        case 0x6804:
            // Enable starfield
            setOutputFlipFlop( StarsEnabled, b & 0x01 );
            break;
        case 0x6806:
            // Flip screen horizontally
            setOutputFlipFlop( FlipScreenX, b & 0x01 );
            break;
        case 0x6807:
            // Flip screen vertically
            setOutputFlipFlop( FlipScreenY, b & 0x01 );
            break;
        case 0x8100:
        case 0x8101:
        case 0x8102:
        case 0x8103:
            // 8255 chip #1
            break;
        case 0x8200:
            // 8255 chip #2 port A/AY-3-8910 #2 port A (sound command)
            sound_command_ = b;
            break;
        case 0x8201:
            // 8255 chip #2 port A/AY-3-8910 #2 port B (interrupt trigger on audio CPU)
            if( (output_devices_ & SoundInterrupt) && (b == 0) ) {
                setOutputFlipFlop( HaveSoundInterrupt, 1 );
            }
            setOutputFlipFlop( SoundInterrupt, b & 0x08 );
            break;
        case 0x8202:
        case 0x8203:
            // 8255 chip #2
            break;
        }
    }
}

void Scramble::reset()
{
    main_board_->reset();
    sound_board_.reset();
}

void Scramble::run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate )
{
    if( refresh_roms_ ) {
        onVideoROMsChanged();
        refresh_roms_ = false;
    }

    // Run the main CPU
    main_board_->run();

    // Run the sound CPU
    sound_board_.soundChip(1)->setRegister( AY_3_8910::PortA, main_board_->sound_command_ );
    
    if( main_board_->output_devices_ & HaveSoundInterrupt ) {
        sound_board_.interrupt( 0x00 );
    }

    sound_board_.run();

    sound_board_.playSound( SoundCpuCyclesPerFrame, frame->getMixer(), samplesPerFrame, samplingRate );

    // Update the starfield state
    starfield_blink_timer_ += MicrosecondsPerFrame;

    if( starfield_blink_timer_ >= starfield_blink_period_ ) {
        starfield_blink_timer_ -= starfield_blink_period_;
        starfield_blink_state_++;
    }

    starfield_scroll_pos_++;
    if( (main_board_->output_devices_ & StarsEnabled) == 0 ) {
        starfield_scroll_pos_ = 0;
    }

    // Render the video
    frame->setVideo( renderVideo() );
}

void Scramble::decodeChar( const unsigned char * src, TBitBlock * bb, int ox, int oy, int planes, unsigned plane_size )
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

void Scramble::decodeSprite( const unsigned char * src, TBitBlock * bb, int ox, int oy )
{
    decodeChar( src,     bb, ox + 8, oy );
    decodeChar( src + 8, bb, ox + 8, oy + 8 );
    decodeChar( src +16, bb, ox + 0, oy );
    decodeChar( src +24, bb, ox + 0, oy + 8 );
}

void Scramble::drawStarfield()
{
    for( int i=0; i<NumOfStars; i++ ) {
        int x = starfield_[i].x;
        int y = starfield_[i].y / 2;

        if( (x & 0x01) ^ ((y >> 3) & 0x01) ) {
            bool plot = true;

            switch( starfield_blink_state_ & 0x03 ) {
            case 0:
                plot = (starfield_[i].color & 0x01) != 0;
                break;
            case 1:
                plot = (starfield_[i].color & 0x04) != 0;
                break;
            case 2:
                plot = (starfield_[i].x & 0x02) != 0;
            case 3:
                // Plot always
                break;
            }

            if( plot && starfield_[i].color ) {
                if( main_board_->output_devices_ & FlipScreenX ) x = 255-x;
                if( main_board_->output_devices_ & FlipScreenY ) y = 255-y;

                screen()->bits()->setPixel( x-16, y, StarsPaletteIndex+starfield_[i].color );
            }
		}
    }
}

void Scramble::drawBullets( unsigned char * bullet_ram, unsigned mode )
{
    for( int bullet=0; bullet<8; bullet++ ) {
        unsigned char * bullet_data = bullet_ram + bullet*4;

        int x = bullet_data[1];
        int y = 255 - bullet_data[3];

        if( mode & opFlipX ) x = 255 - x;
        if( mode & opFlipY ) y = y - 2;

        y -= 6;

        screen()->bits()->setPixel( x-16, y, BulletPaletteIndex );
	}
}

TBitmapIndexed * Scramble::renderVideo()
{
    unsigned char * video = main_board_->ram_ + 0x4800;
    unsigned char * attr = main_board_->ram_ + 0x5000;

    TBltAddSrcZeroTrans         blitter_f(0);
    TBltAddSrcZeroTransReverse  blitter_r(0);

    unsigned mode = opAdd |
        (main_board_->output_devices_ & FlipScreenX ? opFlipX : 0) | 
        (main_board_->output_devices_ & FlipScreenY ? opFlipY : 0);

    TBltAddSrcZeroTrans * blitter = (mode & opFlipX) ? &blitter_r : &blitter_f;

    // Set the background color...
    screen()->bits()->fill( main_board_->output_devices_ & BackgroundEnabled ? BackgroundPaletteIndex : 0 );

    // ...then the stars...
    if( main_board_->output_devices_ & StarsEnabled ) {
        drawStarfield();
    }

    // ...then draw the tiles...
    for( int y=0; y<ScreenHeightChars; y++ ) {
        unsigned scroll = attr[ y*2 ];
        unsigned color = attr[ y*2 + 1 ] & 7;

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

    // ...then the bullets...
    drawBullets( main_board_->ram_ + 0x5060, mode );

    // ...and finally the sprites
    for( int i=7; i>=0; i-- ) {
        unsigned char * sram = main_board_->ram_ + 0x5040 + i*4;

        int y = sram[3] + 1;
        int x = sram[0];

        if( x == 0 ) { 
            continue;
        }

        unsigned s_mode = mode ^ (((sram[1] & 0x40) ? opFlipY : 0) | ((sram[1] & 0x80) ? opFlipX : 0));

        x -= 16;

        if( mode & opFlipX ) x = 208-x;
        if( mode & opFlipY ) y = 239-y;

        blitter = (s_mode & opFlipX) ? &blitter_r : &blitter_f;

        if( i <= 2 ) {
            x++;
        }

        screen()->bits()->copy( x, y, sprite_data_, 0, (sram[1] & 0x3F)*16, 16, 16, s_mode, blitter->color((sram[2] & 0x07)*4) );
    }

    return screen();
}

// Update the internal tables after a ROM has changed
void Scramble::onVideoROMsChanged()
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

AmidarOnScramble::AmidarOnScramble() : Scramble( new ScrambleMainBoard() )
{
    // In the current MAME ROM set this version is linked to "amidar"
    // (for the 32 byte color PROM), so we have to declare that too
    static TMachineInfo AmidarInfo = { "amidar", 0, 0, 0, 0, 0, 0, 0 };

    registerDriver( AmidarInfo );

    registerDriver( AmidarOnScrambleInfo );
}

bool AmidarOnScramble::initialize( TMachineDriverInfo * info )
{
    Scramble::initialize( info );

    resourceHandler()->replace( Ef2D, "am2d",        0x00000000 );
    resourceHandler()->replace( Ef2E, "am2e",        0x00000000 );
    resourceHandler()->replace( Ef2F, "am2f",        0x00000000 );
    resourceHandler()->replace( Ef2H, "am2h",        0x00000000 );
    resourceHandler()->replace( Ef2J, "am2j",        0x00000000 );
    resourceHandler()->replace( Ef2L, "am2l",        0x00000000 );
    resourceHandler()->replace( Ef2M, "am2m",        0x00000000 );
    resourceHandler()->replace( Ef2P, "am2p",        0x00000000 );
    resourceHandler()->replace( Ef5F, "2716.a6",     0x00000000 );
    resourceHandler()->replace( Ef5H, "2716.a5",     0x00000000 );
    resourceHandler()->replace( Ef6E, "amidar.clr",  0x00000000 );

    resourceHandler()->remove( Ef5E );

    // Sound ROMs must be entirely replaced
    resourceHandler()->add( Ef5C, "amidarus.5c", 0x1000, efROM, sound_board_.rom()+0x0000 );
    resourceHandler()->add( Ef5D, "amidarus.5d", 0x1000, efROM, sound_board_.rom()+0x1000 );

    resourceHandler()->assignToMachineDriverInfo( info );

    // Update user interface
    TUserInterface * ui = info->userInterface();

    TUiOption * option = ui->replaceOption( OptLives, "Gorillas per game", 0, 0x03 );
    option->add( "3", 0x03 );
    option->add( "4", 0x02 );
    option->add( "5", 0x01 );
    option->add( S_FreePlay, 0x00 );

    option = ui->replaceOption( OptCoin, S_Coin, 0, 0x06 );
    option->add( "Slot 1: 1C/1P, slot 2: 1C/6P", 0x00 );
    option->add( "Slot 1: 2C/1P, slot 2: 1C/3P", 0x02 );
    option->add( S_Unused, 0x04 );
    option->add( S_Unused, 0x06 );

    optionHandler()->resynch( ui );

    // Joystick is 4-way
    joystickHandler(0)->setMode( jm4Way );
    joystickHandler(1)->setMode( jm4Way );

    return true;
}

Atlantis::Atlantis() : Scramble( new ScrambleMainBoard() )
{
    registerDriver( AtlantisInfo );
}

bool Atlantis::initialize( TMachineDriverInfo * info )
{
    Scramble::initialize( info );

    resourceHandler()->replace( Ef2D, "2c", 0x00000000 );
    resourceHandler()->replace( Ef2E, "2e", 0x00000000 );
    resourceHandler()->replace( Ef2F, "2f", 0x00000000 );
    resourceHandler()->replace( Ef2H, "2h", 0x00000000 );
    resourceHandler()->replace( Ef2J, "2j", 0x00000000 );
    resourceHandler()->replace( Ef2L, "2l", 0x00000000 );
    resourceHandler()->replace( Ef5F, "5f", 0x00000000 );
    resourceHandler()->replace( Ef5H, "5h", 0x00000000 );

    resourceHandler()->remove( Ef2M );
    resourceHandler()->remove( Ef2P );

    resourceHandler()->assignToMachineDriverInfo( info );

    // Update user interface
    TUserInterface * ui = info->userInterface();

    TUiOption * option = ui->replaceOption( OptLives, S_Lives, 0, 0x02 );
    option->add( "3", 0x02 );
    option->add( "5", 0x00 );

    option = ui->replaceOption( OptCabinet, S_Cabinet, 0, 0x01 );
    option->add( S_Upright, 0x00 );
    option->add( S_Cocktail, 0x01 );

    option = ui->replaceOption( OptCoin, S_Coin, 0, 0x0E );
    option->add( "Slot 1: 1C/6P, slot 2: 1C/1P", 0x00 );
    option->add( "Slot 1: 1C/3P, slot 2: 2C/1P", 0x02 );
    option->add( "Freeplay (99 credits)", 0x04 );

    optionHandler()->clear();

    optionHandler()->add( &main_board_->port1_, ui, OptLives, OptCabinet );
    optionHandler()->add( &main_board_->port2_, ui, OptCoin );

    return true;
}

TheEnd::TheEnd() : Scramble( new ScrambleMainBoard() )
{
    registerDriver( TheEndInfo );
}

bool TheEnd::initialize( TMachineDriverInfo * info )
{
    Scramble::initialize( info );

    resourceHandler()->replace( Ef2D, "ic13_1t.bin", 0x00000000 );
    resourceHandler()->replace( Ef2E, "ic14_2t.bin", 0x00000000 );
    resourceHandler()->replace( Ef2F, "ic15_3t.bin", 0x00000000 );
    resourceHandler()->replace( Ef2H, "ic16_4t.bin", 0x00000000 );
    resourceHandler()->replace( Ef2J, "ic17_5t.bin", 0x00000000 );
    resourceHandler()->replace( Ef2L, "ic18_6t.bin", 0x00000000 );
    resourceHandler()->replace( Ef5C, "ic56_1.bin", 0x00000000 );
    resourceHandler()->replace( Ef5D, "ic55_2.bin", 0x00000000 );
    resourceHandler()->replace( Ef5F, "ic30_2c.bin", 0x00000000 );
    resourceHandler()->replace( Ef5H, "ic31_1c.bin", 0x00000000 );
    resourceHandler()->replace( Ef6E, "6331-1j.86", 0x00000000 );

    resourceHandler()->remove( Ef2M );
    resourceHandler()->remove( Ef2P );
    resourceHandler()->remove( Ef5E );

    resourceHandler()->assignToMachineDriverInfo( info );

    // Update user interface
    TUserInterface * ui = info->userInterface();

    TUiOption * option = ui->replaceOption( OptCoin, S_Coin, 0, 0x06 );
    option->add( S_1Coin1Play, 0x00 );
    option->add( S_2Coin1Play, 0x02 );
    option->add( S_3Coin1Play, 0x04 );
    option->add( S_1Coin2Play, 0x06 );

    return true;
}

void TheEnd::drawStarfield()
{
    for( int i=0; i<NumOfStars; i++ ) {
        int y = starfield_[i].y + starfield_scroll_pos_;
        int x = (starfield_[i].x + (y >> 9)) & 0xFF;

        y = (y & 0x1FF) / 2;

        if( (x & 0x01) ^ ((y >> 3) & 0x01) ) {
            if( main_board_->output_devices_ & FlipScreenX ) x = 255-x;
            if( main_board_->output_devices_ & FlipScreenY ) y = 255-y;

            screen()->bits()->setPixel( x-16, y, StarsPaletteIndex+starfield_[i].color );
		}
    }
}

void TheEnd::drawBullets( unsigned char * bullet_ram, unsigned mode )
{
    for( int bullet=0; bullet<8; bullet++ ) {
        unsigned char * bullet_data = bullet_ram + bullet*4;

        int x = bullet_data[1];
        int y = 255 - bullet_data[3];

        if( mode & opFlipX ) x = 255 - x;
        if( mode & opFlipY ) y = y - 1;

        for( int i=0; i<4; i++ ) {
            y--;

            if( y >= 0 ) {
                screen()->bits()->setPixel( x-16, y, BulletPaletteIndex );
            }
        }
    }
}

unsigned char MarsMainBoard::readByte( unsigned addr )
{
    addr &= 0xFFFF;

    // For performance reasons, it's better to handle this very common
    // case here, rather than delegating to the parent
    if( addr < sizeof(ram_) )
        return ram_[addr];

    // Remap 8255 ports if needed
    if( addr >= 0x8100 && addr <= 0x810F ) {
        addr = 0x8100 + ((addr >> 2) & 0x02) + ((addr >> 1) & 0x01);
    }
    else if( addr >= 0x8200 && addr <= 0x820F ) {
        addr = 0x8200 + ((addr >> 2) & 0x02) + ((addr >> 1) & 0x01);

        if( addr == 0x8202 ) {
            // Extra port for player 2 input
            return port3_;
        }
    }

    // Let parent do the work
    return ScrambleMainBoard::readByte( addr );
}

void MarsMainBoard::writeByte( unsigned addr, unsigned char b )
{
    addr &= 0xFFFF;

    if( addr < 0x4000 ) {
        // This is a ROM address, do not write into it!
    }
    else if( addr < sizeof(ram_) ) {
        ram_[addr] = b;
        if( addr >= 0x4800 && addr <= 0x4BFF ) {
            ram_[addr+0x400] = b;
        }
    }
    else if( addr >= 0x8100 && addr <= 0x810F ) {
        // Remap 8255 port and let parent handle it
        ScrambleMainBoard::writeByte( 0x8100 + ((addr >> 2) & 0x02) + ((addr >> 1) & 0x01), b );
    }
    else if( addr >= 0x8200 && addr <= 0x820F ) {
        // Remap 8255 port and let parent handle it
        ScrambleMainBoard::writeByte( 0x8200 + ((addr >> 2) & 0x02) + ((addr >> 1) & 0x01), b );
    }
    else { 
        // Memory mapped ports
        switch( addr ) {
        case 0x6800:
            // Increment coin meter
            if( (output_devices_ & CoinMeter) && (b == 0) ) {
                coin_counter_++;
            }
            setOutputFlipFlop( CoinMeter, b & 0x01 );
            break;
        case 0x6801:
            // Enable starfield
            setOutputFlipFlop( StarsEnabled, b & 0x01 );
            break;
        case 0x6802:
            // Interrupt enable
            setOutputFlipFlop( InterruptEnabled, b & 0x01 );
            break;
        case 0x6808:
            // Increment coin meter
            if( (output_devices_ & CoinMeter) && (b == 0) ) {
                coin_counter_++;
            }
            setOutputFlipFlop( CoinMeter, b & 0x01 );
            break;
        case 0x6809:
            // Flip screen horizontally
            setOutputFlipFlop( FlipScreenX, b & 0x01 );
            break;
        case 0x680B:
            // Flip screen vertically
            setOutputFlipFlop( FlipScreenY, b & 0x01 );
            break;
        }
    }
}

Mars::Mars() : Scramble( new MarsMainBoard() )
{
    registerDriver( MarsInfo );
}

bool Mars::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    unsigned char rombuf[0x800];

    if( (id >= Ef2D) && (id <= Ef2L) ) {
        // Address lines of the main CPU are scrambled, fix them
        assert( len = sizeof(rombuf) );

        for( unsigned i=0; i<len; i ++ ) {
            unsigned j = i & 0x0F;
            unsigned o = ((j & 0x08) >> 2) | ((j & 0x04) << 1) | ((j & 0x02) >> 1) | ((j & 0x01) << 2);

            rombuf[i] = buf[ (i & 0xFFF0) | o ];
        }

        buf = rombuf;
    }

    return Scramble::setResourceFile( id, buf, len );
}

bool Mars::initialize( TMachineDriverInfo * info )
{
    Scramble::initialize( info );

    resourceHandler()->replace( Ef2D, "u26.3", 0x00000000 );
    resourceHandler()->replace( Ef2E, "u56.4", 0x00000000 );
    resourceHandler()->replace( Ef2F, "u69.5", 0x00000000 );
    resourceHandler()->replace( Ef2H, "u98.6", 0x00000000 );
    resourceHandler()->replace( Ef2J, "u114.7", 0x00000000 );
    resourceHandler()->replace( Ef2L, "u133.8", 0x00000000 );

    resourceHandler()->remove( Ef2M );
    resourceHandler()->remove( Ef2P );

    resourceHandler()->replace( Ef5C, "u39.9", 0x00000000 );
    resourceHandler()->replace( Ef5D, "u51.10", 0x00000000 );
    resourceHandler()->replace( Ef5E, "u78.11", 0x00000000 );

    resourceHandler()->replace( Ef5F, "u72.1", 0x00000000 );
    resourceHandler()->replace( Ef5H, "u101.2", 0x00000000 );

    resourceHandler()->assignToMachineDriverInfo( info );

    // Setup user interface
    TUserInterface * ui = info->userInterface();

    ui->clear();

    TUiOption * option;
    TUiOptionGroup * group = ui->addGroup( S_DSW_1, dtDipSwitch );

    option = group->add( OptLives, S_Lives, 0, 0x08 );
    option->add( "3", 0x08 );
    option->add( S_FreePlay, 0x00 );

    option = group->add( OptCoin, "Coin A", 0, 0x02 );
    option->add( S_1Coin1Play, 0x00 );
    option->add( S_2Coin1Play, 0x02 );

    option = group->add( OptCoin2, "Coin B", 0, 0x01 );
    option->add( S_1Coin3Play, 0x00 );
    option->add( S_2Coin5Play, 0x01 );

    option = group->add( OptCabinet, S_Cabinet, 0, 0x02 );
    option->add( S_Upright, 0x00 );
    option->add( S_Cocktail, 0x02 );

    optionHandler()->clear();

    optionHandler()->add( &main_board_->port1_, ui, OptCoin, OptCoin2 );
    optionHandler()->add( &main_board_->port2_, ui, OptLives, OptCabinet );

    // Fix controls
    MarsMainBoard * board = (MarsMainBoard *) main_board_;

    eventHandler()->add( idKeyService1,  ptInverted, &main_board_->port0_, 0x02 );

    eventHandler()->add( idKeyP1Action1, ptInverted, &main_board_->port0_, 0x08 );
    eventHandler()->add( idKeyP1Action2, ptInverted, &main_board_->port2_, 0x80 );
    eventHandler()->add( idKeyP1Action3, ptInverted, &main_board_->port2_, 0x20 );
    eventHandler()->add( idKeyP1Action4, ptInverted, &main_board_->port0_, 0x04 );

    eventHandler()->add( idKeyP2Action1, ptInverted, &main_board_->port1_, 0x08 );
    eventHandler()->add( idKeyP2Action2, ptInverted, &board->port3_, 0x80 );
    eventHandler()->add( idKeyP2Action3, ptInverted, &board->port3_, 0x20 );
    eventHandler()->add( idKeyP2Action4, ptInverted, &main_board_->port1_, 0x04 );

    // Add a second joystick for firing (aliases the action keys)
    setJoystickHandler( 2, new TJoystickToPortHandler(idJoyP1Joystick2, ptInverted, jm8Way) );
    joystickHandler(2)->setPort( jpLeft,  &main_board_->port0_, 0x08 );
    joystickHandler(2)->setPort( jpRight, &main_board_->port0_, 0x04 );
    joystickHandler(2)->setPort( jpUp,    &main_board_->port2_, 0x80 );
    joystickHandler(2)->setPort( jpDown,  &main_board_->port2_, 0x20 );

    setJoystickHandler( 3, new TJoystickToPortHandler(idJoyP2Joystick2, ptInverted, jm8Way) );
    joystickHandler(3)->setPort( jpLeft,  &main_board_->port1_, 0x08 );
    joystickHandler(3)->setPort( jpRight, &main_board_->port1_, 0x04 );
    joystickHandler(3)->setPort( jpUp,    &board->port3_, 0x80 );
    joystickHandler(3)->setPort( jpDown,  &board->port3_, 0x20 );

    // Initialize the extra port
    board->port3_ = 0xFF;

    return true;
}
