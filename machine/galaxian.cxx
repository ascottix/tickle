/*
    Galaxian arcade machine emulator

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "galaxian.h"

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
    MicrosecondsPerFrame    = 1000000 / VideoFrequency,
    CpuCyclesPerFrame       = CpuClock / VideoFrequency,
    CpuCyclesAfterInterrupt = 2500,     // Between main CPU NMI and VBLANK
};

enum { 
    Ef7F, Ef7G, Ef7J, Ef7K, Ef7L, Ef8D, Ef8E, Ef8F, Ef1H, Ef1I, Ef1J, Ef1K, Ef6L,
    OptLives, OptCabinet, OptTest, OptCoin, OptBonus, OptLanguage, OptCoin2
};

// Machine info
static TMachineInfo GalaxianInfo = { 
    "galaxian", "Galaxian", S_Namco, 1979, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo WarOfBugsInfo = { 
    "warofbug", "War of the Bugs", S_Armenia, 1981, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};
 
static TMachineInfo MoonCrestaInfo = { 
    "mooncrst", "Moon Cresta", S_Nichibutsu, 1980, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};
 
static TMachineInfo UniWarSInfo = { 
    "uniwars", "UniWar S", S_Irem, 1980, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo GingateikokuInfo = { 
    "gteikoku", "Gingateikoku No Gyakushu", S_Irem, 1980, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo BlackHoleInfo = { 
    "blkhole", "Black Hole", "TDS", 1981, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TGameRegistrationHandler reg1( &GalaxianInfo, Galaxian::createInstance );
static TGameRegistrationHandler reg2( &WarOfBugsInfo, WarOfBugs::createInstance );
static TGameRegistrationHandler reg3( &MoonCrestaInfo, MoonCresta::createInstance );
static TGameRegistrationHandler reg4( &UniWarSInfo, UniWarS::createInstance );
static TGameRegistrationHandler reg5( &GingateikokuInfo, Gingateikoku::createInstance );
static TGameRegistrationHandler reg6( &BlackHoleInfo, BlackHole::createInstance );

bool Galaxian::initialize( TMachineDriverInfo * info )
{
    // Declare resources
    resourceHandler()->add( Ef7F, "galmidw.u", 0x800, efROM, main_board_->rom_+0x0000 );
    resourceHandler()->add( Ef7G, "galmidw.v", 0x800, efROM, main_board_->rom_+0x0800 );
    resourceHandler()->add( Ef7J, "galmidw.w", 0x800, efROM, main_board_->rom_+0x1000 );
    resourceHandler()->add( Ef7K, "galmidw.y", 0x800, efROM, main_board_->rom_+0x1800 );
    resourceHandler()->add( Ef7L,        "7l", 0x800, efROM, main_board_->rom_+0x2000 );

    resourceHandler()->add( Ef1H,    "1h.bin", 0x800, efVideoROM, video_rom_+0x0000 );
    resourceHandler()->add( Ef1K,    "1k.bin", 0x800, efVideoROM, video_rom_+0x0800 );

    resourceHandler()->add( Ef6L,    "6l.bpr", 0x20, efPalettePROM, palette_prom_ );

    resourceHandler()->assignToMachineDriverInfo( info );

    // Setup user interface
    TUserInterface * ui = info->userInterface();

    TUiOption * option;
    TUiOptionGroup * group = ui->addGroup( S_DSW_1, dtDipSwitch );

    option = group->add( OptBonus, "Bonus at", 0, 0x03 );
    option->add(  "7000", 0x00 );
    option->add( "10000", 0x01 );
    option->add( "12000", 0x02 );
    option->add( "20000", 0x03 );

    option = group->add( OptLives, S_Lives, 1, 0x04 );
    option->add( "2", 0x00 );
    option->add( "3", 0x04 );

    option = group->add( OptCoin, S_Coin, 0, 0xC0 );
    option->add( S_1Coin1Play, 0x00 );
    option->add( S_2Coin1Play, 0x40 );
    option->add( S_1Coin2Play, 0x80 );
    option->add( S_FreePlay, 0xC0 );

    option = group->add( OptCabinet, S_Cabinet, 0, 0x20 );
    option->add( S_Upright, 0x00 );
    option->add( S_Cocktail, 0x20 );

    option = group->add( OptTest, "Rack mode", 0, 0x40 );
    option->add( S_Normal, 0x00 );
    option->add( S_Test, 0x40 );

    optionHandler()->add( &main_board_->port0_, ui, OptCabinet, OptTest );
    optionHandler()->add( &main_board_->port1_, ui, OptCoin );
    optionHandler()->add( &main_board_->port2_, ui, OptBonus, OptLives );

    return true;
}

bool Galaxian::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    // Need to refresh some internal tables if the video/color ROMs change
    refresh_roms_ |= (id >= Ef1H);

    return 0 == resourceHandler()->handle( id, buf, len );
}

Galaxian::Galaxian( GalaxianMainBoard * board ) :
    char_data_( 8, 8*256*2 ),   // 256 8x8 characters, up to 2 banks
    sprite_data_( 16, 16*64*2 ) // 64 16x16 sprites, up to 2 banks
{
    main_board_ = board;
    refresh_roms_ = false;

    createScreen( ScreenWidth, ScreenHeight, ScreenColors );

    eventHandler()->add( idCoinSlot1,        ptNormal, &main_board_->port0_, 0x01 );
    eventHandler()->add( idCoinSlot2,        ptNormal, &main_board_->port0_, 0x02 );
    eventHandler()->add( idKeyP1Action1,     ptNormal, &main_board_->port0_, 0x10 );
    eventHandler()->add( idKeyP2Action1,     ptNormal, &main_board_->port1_, 0x10 );
    eventHandler()->add( idKeyStartPlayer1,  ptNormal, &main_board_->port1_, 0x01 );
    eventHandler()->add( idKeyStartPlayer2,  ptNormal, &main_board_->port1_, 0x02 );

    eventHandler()->add( idKeyService1,      ptNormal, &main_board_->port0_, 0x80 );

    setJoystickHandler( 0, new TJoystickToPortHandler(idJoyP1Joystick1, ptNormal, jm4Way) );
    joystickHandler(0)->setPort( jpLeft,  &main_board_->port0_, 0x04 );
    joystickHandler(0)->setPort( jpRight, &main_board_->port0_, 0x08 );

    setJoystickHandler( 1, new TJoystickToPortHandler(idJoyP2Joystick1, ptNormal, jm4Way) );
    joystickHandler(1)->setPort( jpLeft,  &main_board_->port1_, 0x04 );
    joystickHandler(1)->setPort( jpRight, &main_board_->port1_, 0x08 );

    registerDriver( GalaxianInfo );

    // Setup palette (non-PROM colors)
    TPalette * palette = screen()->palette();

    palette->setColor( BackgroundPaletteIndex, TPalette::encodeColor(0x00,0x00,0x55) ); // Blue
    palette->setColor( BulletPaletteIndex+0, TPalette::encodeColor(0xFF,0xFF,0x00) ); // Yellow
    palette->setColor( BulletPaletteIndex+1, TPalette::encodeColor(0xFF,0xFF,0xFF) ); // White

    unsigned char starfieldColors[4] = { 0x00, 0x66, 0x99, 0xFF };

    for( int i=0; i<64; i++ ) {
        palette->setColor( i + StarsPaletteIndex, TPalette::encodeColor(
            starfieldColors[ (i >> 0) & 0x03 ],
            starfieldColors[ (i >> 2) & 0x03 ],
            starfieldColors[ (i >> 4) & 0x03 ] ) );
    }

    // Generate starfield (setup and the blinking algorithm are
    // from the MAME driver in galaxian.c)
    int stars = 0;
    unsigned generator = 0;

    for( int y=0; y<512; y++ ) {
        for( int x=0; x<256; x++ ) {
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

    // Setup graphics ROM parameters
    low_sprite_offset_ = 1; // First sprites are offset 1 pixel
    gfx_banks_ = 1;
    gfx_plane_size_ = 0x800;
}

GalaxianMainBoard::GalaxianMainBoard()
{
    // Initialize the CPU and the RAM
    cpu_ = new Z80( *this );
    memset( rom_, 0, sizeof(rom_) );

    // Initialize the board variables
    port0_ = 0x00;
    port1_ = 0x00;
    port2_ = 0x00;

    coin_counter_ = 0;

    bank_select_[0] = 0;
    bank_select_[1] = 0;
    bank_select_[2] = 0;

    // Initialize the memory mappings
    addr_rom_end_           = 0x4000;
    addr_ram_               = 0x4000;
    addr_ram_end_           = addr_ram_ + sizeof(ram_);
    addr_video_ram_         = 0x5000;
    addr_video_ram_mirror_  = 0x5400;
    addr_special_ram_       = 0x5800;
    addr_port0_             = 0x6000;
    addr_port1_             = 0x6800;
    addr_port2_             = 0x7000;
    addr_watchdog_          = 0x7800;
    addr_latch_9l_          = 0x6000;
    addr_latch_9m_          = 0x6800;
    addr_interrupt_enable_  = 0x7001;
    addr_starfield_enable_  = 0x7004;
    addr_flip_x_            = 0x7006;
    addr_flip_y_            = 0x7007;
    addr_tone_pitch_        = 0x7800;

    // Reset the machine
    reset();
}

void GalaxianMainBoard::reset()
{
    cpu_->reset();
    output_devices_ = 0;
}

unsigned GalaxianMainBoard::now()
{
    return (cpu_->getCycles() * current_frame_size_ / CpuCyclesPerFrame);
}

void GalaxianMainBoard::run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate )
{
    current_frame_ = frame;
    current_frame_size_ = samplesPerFrame;

    // Reset the sound system
    soundboard_.startFrame();

    // Run the CPU
    setOutputFlipFlop( HaveSoundInterrupt, 0 );

    cpu_->setCycles( 0 );

    cpu_->run( CpuCyclesPerFrame - CpuCyclesAfterInterrupt );

    // If interrupts are enabled, force a CPU interrupt with the vector
    // set by the program
    if( output_devices_ & InterruptEnabled ) {
        cpu_->nmi();
    }

    cpu_->run( CpuCyclesAfterInterrupt );

    // Bring the sound emulation up to date
    soundboard_.endFrame( samplesPerFrame );
}

void GalaxianMainBoard::setOutputFlipFlop( unsigned char bit, unsigned char value )
{
    if( value ) {
        output_devices_ |= bit;
    }
    else {
        output_devices_ &= ~bit;
    }
}

static inline bool inRange( unsigned addr, unsigned mem_start, unsigned mem_size )
{
    return (addr >= mem_start) && (addr < (mem_start+mem_size));
}

unsigned char GalaxianMainBoard::readByte( unsigned addr ) 
{
    addr &= 0xFFFF;

    if( addr < addr_rom_end_ ) {
        return rom_[ addr ];
    }
    else if( addr >= addr_ram_ && addr < addr_ram_end_ ) {
        return ram_[ addr - addr_ram_ ];
    }
    else if( inRange( addr, addr_video_ram_, sizeof(video_ram_) ) ) {
        return video_ram_[ addr - addr_video_ram_ ];
    }
    else if( inRange( addr, addr_video_ram_mirror_, sizeof(video_ram_) ) ) {
        return video_ram_[ addr - addr_video_ram_mirror_ ];
    }
    else if( inRange( addr, addr_special_ram_, sizeof(special_ram_) ) ) {
        return special_ram_[ addr - addr_special_ram_ ];
    }
    else if( addr == addr_port0_ ) {
        return port0_;
    }
    else if( addr == addr_port1_ ) {
        return port1_;
    }
    else if( addr == addr_port2_ ) {
        return port2_;
    }
    else if( addr == addr_watchdog_ ) {
        // Reset watchdog
    }

    return 0;
}

void GalaxianMainBoard::writeByte( unsigned addr, unsigned char b )
{
    addr &= 0xFFFF;

    if( addr < addr_ram_ ) {
        // ROM or unmapped memory
    }
    else if( addr < addr_ram_end_ ) {
        ram_[ addr - addr_ram_ ] = b;
    }
    else if( inRange( addr, addr_video_ram_, sizeof(video_ram_) ) ) {
        video_ram_[ addr - addr_video_ram_ ] = b;
    }
    else if( inRange( addr, addr_special_ram_, sizeof(special_ram_) ) ) {
        special_ram_[ addr - addr_special_ram_ ] = b;
    }
    else if( inRange( addr, addr_latch_9l_, 8 ) ) {
        addr -= addr_latch_9l_;
        if( addr >= 4 ) {
            soundboard_.writeToLatch9L( now(), addr, b );
        }
        else {
            onWriteLatch9L( addr, b );
        }
    }
    else if( inRange( addr, addr_latch_9m_, 8 ) ) {
        soundboard_.writeToLatch9M( now(), addr - addr_latch_9m_, b );
    }
    else if( addr == addr_interrupt_enable_ ) {
        setOutputFlipFlop( InterruptEnabled, b & 0x01 );
    }
    else if( addr == addr_starfield_enable_ ) {
        setOutputFlipFlop( StarsEnabled, b & 0x01 );
    }
    else if( addr == addr_flip_x_ ) {
        setOutputFlipFlop( FlipScreenX, b & 0x01 );
    }
    else if( addr == addr_flip_y_ ) {
        setOutputFlipFlop( FlipScreenY, b & 0x01 );
    }
    else if( addr == addr_tone_pitch_ ) {
        soundboard_.setPitch( now(), b );
    }
}

void GalaxianMainBoard::onWriteLatch9L( unsigned addr, unsigned char b )
{
    switch( addr ) {
        case 0:
            // Player 1 start light
            break;
        case 1:
            // Player 2 start light
            break;
        case 2:
            // Coin lockout
            break;
        case 3:
            // Coin counter
            if( (output_devices_ & CoinMeter) && (b == 0) ) {
                coin_counter_++;
            }
            setOutputFlipFlop( CoinMeter, b & 0x01 );
            break;
    }
}

void Galaxian::reset()
{
    main_board_->reset();
}

void Galaxian::run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate )
{
    if( refresh_roms_ ) {
        onVideoROMsChanged();
        refresh_roms_ = false;
    }

    // Run the main CPU
    main_board_->run( frame, samplesPerFrame, samplingRate );

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

    // Render the sounds
    int voices = 5;

    TMixerBuffer * mixer_buffer = frame->getMixer()->getBuffer( chMono, samplesPerFrame, voices );
    int * dataBuffer = mixer_buffer->data();

    // TODO: there is an important caveat here!!! The sampling rate of the ASE
    // system is independent of the sound sampling rate. At present, however,
    // I have set both to the same value so I can mix directly into the sound buffer.

    main_board_->soundboard_.playFrame( dataBuffer, samplesPerFrame );
}

void Galaxian::decodeChar( const unsigned char * src, TBitBlock * bb, int ox, int oy )
{
    int planes = 2;
    unsigned plane_size = gfx_plane_size_;

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

void Galaxian::decodeSprite( const unsigned char * src, TBitBlock * bb, int ox, int oy )
{
    decodeChar( src,     bb, ox + 8, oy     );
    decodeChar( src + 8, bb, ox + 8, oy + 8 );
    decodeChar( src +16, bb, ox + 0, oy     );
    decodeChar( src +24, bb, ox + 0, oy + 8 );
}

// Update the internal tables after a ROM has changed
void Galaxian::onVideoROMsChanged()
{
    int i;

    // Refresh palette
    for( i=0; i<32; i++ ) {
        palette()->setColor( i, TPalette::decodeByte(palette_prom_[i]) );
    }

    // Decode character set
    for( i=0; i<256*gfx_banks_; i++ ) {
        decodeChar( video_rom_ + 8*i, &char_data_, 0, i*8 );
    }

    // Decode sprite set
    for( i=0; i<64*gfx_banks_; i++ ) {
        decodeSprite( video_rom_ + 32*i, &sprite_data_, 0, 16*i );
    }
}

void Galaxian::drawStarfield()
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

unsigned Galaxian::translateCharCode( unsigned code )
{
    return code;
}

unsigned Galaxian::translateSpriteCode( unsigned code )
{
    return code;
}

TBitmapIndexed * Galaxian::renderVideo()
{
    unsigned char * video = main_board_->video_ram_;
    unsigned char * attr = main_board_->special_ram_;

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
                if( mode & opFlipY ) cy = 248 - cy;

                unsigned code = translateCharCode( video[offset] );

                screen()->bits()->copy( cx, cy, char_data_, 0, code*8, 8, 8, mode, blitter->color(color*4) );
            }
        }
    }

    // ...then the bullets...
    for( int bullet=0; bullet<8; bullet++ ) {
        unsigned char * bullet_data = main_board_->special_ram_ + 0x60 + bullet*4;

        int x = bullet_data[1] - 16;
        int y = 255 - bullet_data[3];

        if( mode & opFlipX ) x = 223 - x;
        if( mode & opFlipY ) y--;

        int color = (bullet == 7) ? BulletPaletteIndex : BulletPaletteIndex+1;

        for( int i=0; i<4; i++ ) {
            y--;

            if( y >= 0 ) {
                screen()->bits()->setPixel( x, y, color );
            }
        }
	}

    // ...and finally the sprites
    for( int i=7; i>=0; i-- ) {
        unsigned char * sram = main_board_->special_ram_ + 0x40 + i*4;

        int y = sram[3] + 1;
        int x = sram[0] - 16;

        if( y < 8 ) continue;

        unsigned s_mode = mode ^ (((sram[1] & 0x40) ? opFlipY : 0) | ((sram[1] & 0x80) ? opFlipX : 0));

        if( mode & opFlipX ) x = 208-x;
        if( mode & opFlipY ) y = 240-y;

        blitter = (s_mode & opFlipX) ? &blitter_r : &blitter_f;

        if( i <= 2 ) {
            x += low_sprite_offset_;
        }

        unsigned code = translateSpriteCode( sram[1] & 0x3F );

        screen()->bits()->copy( x, y, sprite_data_, 0, code*16, 16, 16, s_mode, blitter->color((sram[2] & 0x07)*4) );
    }

    return screen();
}

WarOfBugs::WarOfBugs() :
    Galaxian( new GalaxianMainBoard )
{
    registerDriver( WarOfBugsInfo );
}

bool WarOfBugs::initialize( TMachineDriverInfo * info )
{
    Galaxian::initialize( info );

    resourceHandler()->add( Ef7F, "warofbug.u", 0x800, efROM, main_board_->rom_+0x0000 );
    resourceHandler()->add( Ef7G, "warofbug.v", 0x800, efROM, main_board_->rom_+0x0800 );
    resourceHandler()->add( Ef7J, "warofbug.w", 0x800, efROM, main_board_->rom_+0x1000 );
    resourceHandler()->add( Ef7K, "warofbug.y", 0x800, efROM, main_board_->rom_+0x1800 );
    resourceHandler()->add( Ef7L, "warofbug.z", 0x800, efROM, main_board_->rom_+0x2000 );

    resourceHandler()->add( Ef1H, "warofbug.1k", 0x800, efROM, video_rom_+0x0000 );
    resourceHandler()->add( Ef1K, "warofbug.1j", 0x800, efROM, video_rom_+0x0800 );

    resourceHandler()->add( Ef6L, "warofbug.clr", 0x20, efPROM, palette_prom_ );

    resourceHandler()->assignToMachineDriverInfo( info );

    // Joystick is 8-way
    joystickHandler(0)->setMode( jm8Way );
    joystickHandler(1)->setMode( jm8Way );

    eventHandler()->remove( idCoinSlot2 );      // Used by P2 Up
    eventHandler()->remove( idKeyService1 );    // Used by P1 Up

    joystickHandler(0)->setPort( jpUp,   &main_board_->port0_, 0x80 );
    joystickHandler(0)->setPort( jpDown, &main_board_->port0_, 0x40 );

    joystickHandler(1)->setPort( jpUp,   &main_board_->port0_, 0x02 );
    joystickHandler(1)->setPort( jpDown, &main_board_->port1_, 0x20 );

    TUserInterface * ui = info->userInterface();

    ui->clear();

    TUiOption * option;
    TUiOptionGroup * group = ui->addGroup( S_DSW_1, dtDipSwitch );

    option = group->add( OptLives, S_Lives, 2, 0x03 );
    option->add( "1", 0x00 );
    option->add( "2", 0x01 );
    option->add( "3", 0x02 );
    option->add( "4", 0x03 );

    option = group->add( OptBonus, "Bonus at", 0, 0x08 );
    option->add( "500000", 0x00 );
    option->add( "750000", 0x08 );

    option = group->add( OptCoin, S_Coin, 0, 0xC0 );
    option->add( S_1Coin1Play, 0x00 );
    option->add( S_2Coin1Play, 0x40 );
    option->add( S_FreePlay, 0xC0 );

    option = group->add( OptCabinet, S_Cabinet, 0, 0x20 );
    option->add( S_Upright, 0x00 );
    option->add( S_Cocktail, 0x20 );

    optionHandler()->clear();

    optionHandler()->add( &main_board_->port0_, ui, OptCabinet );
    optionHandler()->add( &main_board_->port1_, ui, OptCoin );
    optionHandler()->add( &main_board_->port2_, ui, OptBonus, OptLives );
    
    return true;
}

MoonCrestaMainBoard::MoonCrestaMainBoard() : GalaxianMainBoard()
{
    addr_rom_end_           = 0x4000;
    addr_ram_               = 0x8000;
    addr_ram_end_           = addr_ram_ + sizeof(ram_);
    addr_video_ram_         = 0x9000;
    addr_video_ram_mirror_  = 0x9400;
    addr_special_ram_       = 0x9800;
    addr_port0_             = 0xA000;
    addr_port1_             = 0xA800;
    addr_port2_             = 0xB000;
    addr_watchdog_          = 0xB800;
    addr_latch_9l_          = 0xA000;
    addr_latch_9m_          = 0xA800;
    addr_interrupt_enable_  = 0xB000;
    addr_starfield_enable_  = 0xB004;
    addr_flip_x_            = 0xB006;
    addr_flip_y_            = 0xB007;
    addr_tone_pitch_        = 0xB800;
}

void MoonCrestaMainBoard::onWriteLatch9L( unsigned addr, unsigned char b )
{
    if( addr <= 2 ) {
        bank_select_[ addr ] = b & 1;
    }
}

MoonCresta::MoonCresta() :
    Galaxian( new MoonCrestaMainBoard )
{
    low_sprite_offset_ = 0; // No sprite offset here!
    gfx_banks_ = 2;
    gfx_plane_size_ = 0x1000;

    registerDriver( MoonCrestaInfo );
}

unsigned MoonCresta::translateCharCode( unsigned code )
{
    if( main_board_->bank_select_[2] && ((code & 0xC0) == 0x80) ) {
        code = (code & 0x3F) | (main_board_->bank_select_[0] << 6) | (main_board_->bank_select_[1] << 7) | 0x100;
    }

    return code;
}

unsigned MoonCresta::translateSpriteCode( unsigned code )
{
    if( main_board_->bank_select_[2] && ((code & 0x30) == 0x20) ) {
        code = (code & 0x0F) | (main_board_->bank_select_[0] << 4) | (main_board_->bank_select_[1] << 5) | 0x40;
    }

    return code;
}

bool MoonCresta::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    unsigned char rombuf[0x800];

    if( (id >= Ef7F) && (id <= Ef8F) ) {
        // Decode ROM
        assert( len = sizeof(rombuf) );

        for( unsigned i=0; i<len; i ++ ) {
            unsigned b = buf[i];

            if( b & 0x02 ) b ^= 0x40;
            if( b & 0x20 ) b ^= 0x04;

            if( (i & 1) == 0 ) {
                b = (b & 0xBB) | ((b & 0x40) >> 4) | ((b & 0x04) << 4);
            }

            rombuf[i] = b;
        }

        buf = rombuf;
    }

    return Galaxian::setResourceFile( id, buf, len );
}

bool MoonCresta::initialize( TMachineDriverInfo * info )
{
    Galaxian::initialize( info );

    resourceHandler()->add( Ef7F, "mc1",    0x800, efROM, main_board_->rom_+0x0000 );
    resourceHandler()->add( Ef7G, "mc2",    0x800, efROM, main_board_->rom_+0x0800 );
    resourceHandler()->add( Ef7J, "mc3",    0x800, efROM, main_board_->rom_+0x1000 );
    resourceHandler()->add( Ef7K, "mc4",    0x800, efROM, main_board_->rom_+0x1800 );
    resourceHandler()->add( Ef7L, "mc5.7r", 0x800, efROM, main_board_->rom_+0x2000 );
    resourceHandler()->add( Ef8D, "mc6.8d", 0x800, efROM, main_board_->rom_+0x2800 );
    resourceHandler()->add( Ef8E, "mc7.8e", 0x800, efROM, main_board_->rom_+0x3000 );
    resourceHandler()->add( Ef8F, "mc8",    0x800, efROM, main_board_->rom_+0x3800 );

    resourceHandler()->add( Ef1H, "mcs_b",  0x800, efVideoROM, video_rom_+0x0000 );
    resourceHandler()->add( Ef1K, "mcs_d",  0x800, efVideoROM, video_rom_+0x0800 );
    resourceHandler()->add( Ef1I, "mcs_a",  0x800, efVideoROM, video_rom_+0x1000 );
    resourceHandler()->add( Ef1J, "mcs_c",  0x800, efVideoROM, video_rom_+0x1800 );

    resourceHandler()->add( Ef6L, "l06_prom.bin",  0x20, efPalettePROM, palette_prom_ );

    resourceHandler()->assignToMachineDriverInfo( info );

    // Event handlers
    eventHandler()->remove( idKeyService1 );    // Not used

    // User interface
    TUserInterface * ui = info->userInterface();

    ui->clear();

    TUiOption * option;
    TUiOptionGroup * group = ui->addGroup( S_DSW_1, dtDipSwitch );

    option = group->add( OptBonus, "Bonus at", 0, 0x40 );
    option->add( "30000", 0x00 );
    option->add( "50000", 0x40 );

    option = group->add( OptLanguage, "Language", 0, 0x80 );
    option->add( "Japanese", 0x00 );
    option->add( "English", 0x80 );

    option = group->add( OptCoin, "Coin slot #1", 0, 0x03 );
    option->add( S_1Coin1Play, 0x00 );
    option->add( S_2Coin1Play, 0x01 );
    option->add( S_3Coin1Play, 0x02 );
    option->add( S_4Coin1Play, 0x03 );

    option = group->add( OptCoin2, "Coin slot #2", 0, 0x0C );
    option->add( S_1Coin1Play, 0x00 );
    option->add( S_1Coin2Play, 0x04 );
    option->add( S_1Coin3Play, 0x08 );
    option->add( S_FreePlay,   0x0C );

    option = group->add( OptCabinet, S_Cabinet, 0, 0x20 );
    option->add( S_Upright, 0x00 );
    option->add( S_Cocktail, 0x20 );

    optionHandler()->clear();

    optionHandler()->add( &main_board_->port0_, ui, OptCabinet );
    optionHandler()->add( &main_board_->port1_, ui, OptBonus, OptLanguage );
    optionHandler()->add( &main_board_->port2_, ui, OptCoin, OptCoin2 );
    
    return true;
}

void UniWarSMainBoard::onWriteLatch9L( unsigned addr, unsigned char b )
{
    if( addr == 2 ) {
        bank_select_[ 0 ] = b & 1;
    }
}

UniWarS::UniWarS() :
    Galaxian( new UniWarSMainBoard )
{
    low_sprite_offset_ = 0; // No sprite offset here!
    gfx_banks_ = 2;
    gfx_plane_size_ = 0x1000;

    registerDriver( UniWarSInfo );
}

unsigned UniWarS::translateCharCode( unsigned code )
{
    return code | ((unsigned)main_board_->bank_select_[0] << 8);
}

unsigned UniWarS::translateSpriteCode( unsigned code )
{
    return code | ((unsigned)main_board_->bank_select_[0] << 6);
}

bool UniWarS::initialize( TMachineDriverInfo * info )
{
    Galaxian::initialize( info );

    resourceHandler()->add( Ef7F, "f07_1a.bin",  0x800, efROM, main_board_->rom_+0x0000 );
    resourceHandler()->add( Ef7G, "h07_2a.bin",  0x800, efROM, main_board_->rom_+0x0800 );
    resourceHandler()->add( Ef7J, "k07_3a.bin",  0x800, efROM, main_board_->rom_+0x1000 );
    resourceHandler()->add( Ef7K, "m07_4a.bin",  0x800, efROM, main_board_->rom_+0x1800 );
    resourceHandler()->add( Ef7L, "d08p_5a.bin", 0x800, efROM, main_board_->rom_+0x2000 );
    resourceHandler()->add( Ef8D, "gg6",         0x800, efROM, main_board_->rom_+0x2800 );
    resourceHandler()->add( Ef8E, "m08p_7a.bin", 0x800, efROM, main_board_->rom_+0x3000 );
    resourceHandler()->add( Ef8F, "n08p_8a.bin", 0x800, efROM, main_board_->rom_+0x3800 );

    resourceHandler()->add( Ef1H, "egg10",     0x800, efROM, video_rom_+0x0000 );
    resourceHandler()->add( Ef1K, "h01_2.bin", 0x800, efROM, video_rom_+0x0800 );
    resourceHandler()->add( Ef1I, "egg9",      0x800, efROM, video_rom_+0x1000 );
    resourceHandler()->add( Ef1J, "k01_2.bin", 0x800, efROM, video_rom_+0x1800 );

    resourceHandler()->add( Ef6L, "uniwars.clr",  0x20, efPROM, palette_prom_ );

    resourceHandler()->assignToMachineDriverInfo( info );

    // User interface
    TUserInterface * ui = info->userInterface();

    ui->renameOptionItem( OptBonus, 0, "4000" );
    ui->renameOptionItem( OptBonus, 1, "5000" );
    ui->renameOptionItem( OptBonus, 2, "7000" );
    ui->renameOptionItem( OptBonus, 3, "None" );
        
    ui->renameOptionItem( OptLives, 0, "3" );
    ui->renameOptionItem( OptLives, 1, "5" );
    ui->setOptionDefault( OptLives, 0, true );

    return true;
}

Gingateikoku::Gingateikoku() :
    UniWarS()
{
    registerDriver( GingateikokuInfo );
}

bool Gingateikoku::initialize( TMachineDriverInfo * info )
{
    UniWarS::initialize( info );

    resourceHandler()->replace( Ef8D, "e08p_6a.bin", 0x00000000 );

    resourceHandler()->replace( Ef1H, "h01_1.bin", 0x00000000 );
    resourceHandler()->replace( Ef1I, "k01_1.bin", 0x00000000 );

    resourceHandler()->replace( Ef6L, "l06_prom.bin", 0x00000000 );

    resourceHandler()->assignToMachineDriverInfo( info );

    return true;
}

BlackHole::BlackHole() : 
    Galaxian( new GalaxianMainBoard )
{
    low_sprite_offset_ = -1;

    registerDriver( BlackHoleInfo );
}

bool BlackHole::initialize( TMachineDriverInfo * info )
{
    Galaxian::initialize( info );

    resourceHandler()->replace( Ef7F, "bh1", 0x00000000 );
    resourceHandler()->replace( Ef7G, "bh2", 0x00000000 );
    resourceHandler()->replace( Ef7J, "bh3", 0x00000000 );
    resourceHandler()->replace( Ef7K, "bh4", 0x00000000 );
    resourceHandler()->replace( Ef7L, "bh5", 0x00000000 );

    resourceHandler()->add( Ef8D, "bh6", 0x800, efROM, main_board_->rom_+0x2800 );

    resourceHandler()->replace( Ef1H, "bh7", 0x00000000 );
    resourceHandler()->replace( Ef1K, "bh8", 0x00000000 );

    resourceHandler()->replace( Ef6L, "6l.bpr", 0x00000000 );

    resourceHandler()->assignToMachineDriverInfo( info );

    // Event handling
    eventHandler()->remove( idCoinSlot2 );      // Not used
    eventHandler()->remove( idKeyService1 );    // Not used

    // User interface
    TUserInterface * ui = info->userInterface();

    ui->clear();

    TUiOption * option;
    TUiOptionGroup * group = ui->addGroup( S_DSW_1, dtDipSwitch );

    option = group->add( OptCoin, S_Coin, 0, 0xC0 );
    option->add( S_1Coin1Play, 0x00 );
    option->add( S_1Coin2Play, 0x40 );
    option->add( S_1Coin3Play, 0x80 );
    option->add( S_2Coin1Play, 0xC0 );

    option = group->add( OptBonus, "Bonus at", 0, 0x01 );
    option->add(  "5000", 0x00 );
    option->add( "10000", 0x01 );

    option = group->add( OptCabinet, S_Cabinet, 0, 0x02 );
    option->add( S_Upright, 0x02 );
    option->add( S_Cocktail, 0x00 );

    optionHandler()->clear();

    optionHandler()->add( &main_board_->port1_, ui, OptCoin );
    optionHandler()->add( &main_board_->port2_, ui, OptCabinet, OptBonus );

    return true;
}
