/*
    Rally X arcade machine emulator

    Copyright (c) 2011 Alessandro Scotti
*/
#include "rallyx.h"

// Hardware info
enum {
    ScreenWidth             = 288,
    ScreenHeight            = 224,
    ScreenColors            = 256,
    VideoFrequency          = 60,
    CpuClock                = 3072000,
    SoundClock              = CpuClock / 32,
    CpuCyclesPerFrame       = CpuClock / VideoFrequency
};

enum {
    Rx1B, Rx1E, Rx1H, Rx1K, Rx8E, Rx11N, Rx8P, Rx8M, Rx3P, Rx8D,
    OptLives, OptBonus, OptCoinage
};

// Machine info
static TMachineInfo RallyXInfo = { 
    "rallyx", "Rally X", S_Namco, 1980, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};
 
static TMachineInfo NewRallyXInfo = { 
    "nrallyx", "New Rally X", S_Namco, 1981, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TGameRegistrationHandler reg1( &RallyXInfo, RallyX::createInstance );
static TGameRegistrationHandler reg2( &NewRallyXInfo, NewRallyX::createInstance );

const AFloat HitFilterGain = 100;
const AFloat HitStreamGain = 25;
const int HitVolume = 150;

bool RallyX::initialize( TMachineDriverInfo * info )
{
    resourceHandler()->add( Rx1B, "1b",         0x1000, efROM, main_board_->rom_+ 0x0000 );
    resourceHandler()->add( Rx1E, "rallyxn.1e", 0x1000, efROM, main_board_->rom_+ 0x1000 );
    resourceHandler()->add( Rx1H, "rallyxn.1h", 0x1000, efROM, main_board_->rom_+ 0x2000 );
    resourceHandler()->add( Rx1K, "rallyxn.1k", 0x1000, efROM, main_board_->rom_+ 0x3000 );

    resourceHandler()->add( Rx8E, "8e", 0x1000, efVideoROM, video_rom_ );

    resourceHandler()->add( Rx11N, "rx1-1.11n",  0x20, efPalettePROM, palette_prom_ );
    resourceHandler()->add( Rx8P,  "rx1-7.8p",  0x100, efPROM, palette_lookup_prom_ );
    resourceHandler()->add( Rx8M,  "rx1-6.8m",  0x100, efPROM, dots_prom_ );
    
    resourceHandler()->add( Rx3P,  "rx1-5.3p",  0x100, efSoundPROM, 0 );

    resourceHandler()->assignToMachineDriverInfo( info );

    // Setup user interface
    TUserInterface * ui = info->userInterface();
    
    TUiOptionGroup * group = ui->addGroup( S_DSW_A, dtDipSwitch );
    
    TUiOption * option = group->add( OptLives, "Difficulty", 1, 0x38 );
    option->add( "Low diff, 2 cars", 0x00 );
    option->add( "Low diff, 3 cars", 0x08 );
    option->add( "Mid diff, 1 car",  0x10 );
    option->add( "Mid diff, 2 cars", 0x18 );
    option->add( "Mid diff, 3 cars", 0x20 );
    option->add( "High diff, 1 car", 0x28 );
    option->add( "High diff, 2 cars",0x30 );
    option->add( "High diff, 3 cars",0x38 );
    
    option = group->add( OptBonus, "Bonus cars", 0, 0x06 );
    option->add( "20000 (2 car game=15000, 1 car game=10000)", 0x02 );
    option->add( "40000 (2 car game=30000, 1 car game=20000)", 0x04 );
    option->add( "60000 (2 car game=40000, 1 car game=30000)", 0x06 );
    
    option = group->add( OptCoinage, S_Coinage, 0, 0xC0 );
    option->add( S_1Coin1Play, 0xC0 );
    option->add( S_2Coin1Play, 0x40 );
    option->add( S_1Coin2Play, 0x80 );
    option->add( S_FreePlay,   0x00 );
    
    optionHandler()->add( &main_board_->dsw_a_, ui, OptLives, OptBonus, OptCoinage );
    
    return true;
}

bool RallyX::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    if( id == Rx3P ) {
        main_board_->sound_chip_.setSoundPROM( buf );
        return true;
    }

    return 0 == resourceHandler()->handle( id, buf, len );
}

RallyX::RallyX( RallyXMainBoard * board ) :
    char_data_( 8, 8*256 ),     // 256 8x8 characters
    dots_data_( 4, 4*8 ),       // 8 4x4 dot characters
    sprite_data_( 16, 16*64 )   // 64 16x16 sprites
{
    main_board_ = board;

    refresh_roms_ = true;

    setJoystickHandler( 0, new TJoystickToPortHandler(idJoyP1Joystick1, ptInverted, jm4Way, &board->port0_, 0x10200804) ); // Down, up, right, left
    setJoystickHandler( 1, new TJoystickToPortHandler(idJoyP2Joystick1, ptInverted, jm4Way, &board->port1_, 0x10200804) );

    eventHandler()->add( idKeyP1Action1,     ptInverted, &main_board_->port0_, 0x02 );
    eventHandler()->add( idKeyP2Action1,     ptInverted, &main_board_->port1_, 0x02 );
    
    eventHandler()->add( idCoinSlot1,        ptInverted, &board->port0_, 0x80 );
    eventHandler()->add( idCoinSlot2,        ptInverted, &board->port1_, 0x80 );
    eventHandler()->add( idKeyStartPlayer1,  ptInverted, &board->port0_, 0x40 );
    eventHandler()->add( idKeyStartPlayer2,  ptInverted, &board->port1_, 0x40 );

    createScreen( ScreenWidth, ScreenHeight, ScreenColors );

    registerDriver( RallyXInfo );
}

RallyXMainBoard::RallyXMainBoard() :
    sound_chip_( SoundClock )
{
    // Initialize the CPU and the RAM
    cpu_ = new Z80( *this );

    memset( rom_, 0, sizeof(rom_) );
    memset( ram_, 0, sizeof(ram_) );
    memset( video_ram_, 0, sizeof(video_ram_) );
    
    // Initialize the board variables
    port0_ = 0xFF;
    port1_ = 0xFF;
    dsw_a_ = 0xC0 + 0x08 + 0x02 + 0x01;

    // Reset the machine
    reset();
    
    a_noise_ = new AWhiteNoise( 8000*6 ); // 4006 LFSR
    a_noise_->setOutput( 3.4, 0.2 );
    a_hit_latch_ = new ALatch; // Actually a 4066 switch in the schematics (no diode)
    a_hit_diode_ = new AClipperLo( *a_hit_latch_, 0.7 );
    a_hit_ = new ACapacitorWithSwitch( *a_hit_diode_, *a_noise_, Kilo(1), Kilo(172), Micro(1) );
    a_hit_filter_ = new AActiveBandPassFilter( *a_hit_, Kilo(150), Kilo(22), Kilo(470), Micro(0.01), Micro(0.01) );
    a_hit_filter_->setGain( HitFilterGain );
    a_rc_filter_1_ = new ALowPassRCFilter( *a_hit_filter_, Kilo(4.7), Micro(0.047) );
}

void RallyXMainBoard::reset()
{
    irq_opcode_ = 0;
    irq_enabled_ = false;
    irq_missed_ = false;
    scroll_x_ = 0;
    scroll_y_ = 0;

    cpu_->reset();
}

void RallyXMainBoard::run()
{
    cpu_->run( CpuCyclesPerFrame );
    
    if( irq_enabled_ ) {
        // If interrupts are disabled at CPU level remember this and fire the interrupt as soon as possible
        irq_missed_ = ! cpu_->interrupt( irq_opcode_ );
    }
}

unsigned char RallyXMainBoard::readByte( unsigned addr ) 
{
    addr &= 0xFFFF;
    
    if( addr < 0x4000 ) {
        return rom_[addr];
    }
    else if( addr >= 0x8000 && addr < 0x9000 ) {
        return video_ram_[ addr-0x8000 ];
    }
    else if( addr >= 0x9800 && addr < 0xA000 ) {
        return ram_[ addr-0x9800 ];
    }
    else if( addr == 0xA000 ) {
        return port0_;
    }
    else if( addr == 0xA080 ) {
        return port1_;
    }
    else if( addr == 0xA100 ) {
        return dsw_a_;
    }
    
    return 0xFF;
}

void RallyXMainBoard::onInterruptsEnabled() {
    if( irq_missed_ ) {
        irq_missed_ = false;
        cpu_->interrupt( irq_opcode_ );
    }
}

void RallyXMainBoard::writeByte( unsigned addr, unsigned char b )
{
    addr &= 0xFFFF;
    
    if( addr >= 0x8000 && addr < 0x9000 ) {
        video_ram_[ addr-0x8000 ] = b;
    }
    else if( addr >= 0x9800 && addr < 0xA000 ) {
        ram_[ addr-0x9800 ] = b;
    }
    else if( addr >= 0xA000 && addr < 0xA010 ) {
        radar_ram_[ addr-0xA000 ] = b;
    }
    else if( addr >= 0xA100 && addr < 0xA120 ) {
        sound_chip_.setRegister( addr-0xA100, b );
    }
    else if( addr == 0xA130 ) {
        scroll_x_ = b;
    }
    else if( addr == 0xA140 ) {
        scroll_y_ = b;
    }
    else switch( addr ) {
    case 0xA080:
        // Watchdog reset
        break;
    case 0xA170:
        break;
    case 0xA180:
        if( b != 0 ) a_hit_latch_->setValue(3.2);
        break;
    case 0xA181:
        irq_enabled_ = (b & 1) != 0;
        irq_missed_ = false;
        break;
    }
}

void RallyXMainBoard::writePort( unsigned addr, unsigned char b )
{
    if( addr == 0 ) {
        irq_opcode_ = b;
        irq_missed_ = false;
    }
}

void RallyX::reset()
{
    main_board_->reset();
}

void RallyX::run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate )
{
    if( refresh_roms_ ) {
        onVideoROMsChanged();
        refresh_roms_ = false;
    }
    
    // Run
    main_board_->run();

    // Sound
    TMixerBuffer * mixer_buffer = frame->getMixer()->getBuffer( chMono, samplesPerFrame, 3 );
    
    main_board_->sound_chip_.setSamplingRate( samplingRate );
    main_board_->sound_chip_.playSound( mixer_buffer->data(), samplesPerFrame );
    
    // Bang
    main_board_->a_rc_filter_1_->resetStream();
    main_board_->a_rc_filter_1_->updateTo( samplesPerFrame );
    main_board_->a_rc_filter_1_->mixStream( mixer_buffer->data(), HitStreamGain, 0, HitVolume ); // Noise must be clipped
    main_board_->a_hit_latch_->setValue(0.3);
    
    // Render the video
    frame->setVideo( renderVideo() );
}

TBitmapIndexed * RallyX::renderVideo()
{
    int scrollx = main_board_->scroll_x_;
    int scrolly = main_board_->scroll_y_;
    
    unsigned char * video_ram = main_board_->video_ram_;
    
    // Draw the background characters...
    screen()->bits()->setClipRegion( 0, 0, 224, 224 );
    
    TBltAddXlat         bg_blitter_f(0,palette_xlat_);
    TBltAddXlatReverse  bg_blitter_r(0,palette_xlat_);
    
    for( int cx=0; cx<32; cx++ ) {
        for( int cy=0; cy<32; cy++ ) {
            // Get the offset of this location in the video RAM
            unsigned offset = 32*cy + cx;
            
            unsigned v = video_ram[offset+0x400];
            unsigned c = video_ram[offset+0xC00];
            
            if( (c & 0x20) != 0)  continue;
            
            unsigned m = 0;
            
            if( (c & 0x40) == 0 ) m |= opFlipX;
            if( (c & 0x80) != 0 ) m |= opFlipY;
            
            TBltAdd * bg_blitter = (m & opFlipX) ? &bg_blitter_r : &bg_blitter_f;
            
            int x = (cx*8 + 256 - scrollx) % 256;
            int y = (cy*8 + 256 - scrolly) % 256 - 16;
            
            c &= 0x3F;
            
            screen()->bits()->copy( x,     y, char_data_, 0, 8*v, 8, 8, m, bg_blitter->color(c*4) );
            screen()->bits()->copy( x-256, y, char_data_, 0, 8*v, 8, 8, m, bg_blitter->color(c*4) ); // For wrap-around
        }
    }
    
    // ...then the radar/score section...
    screen()->bits()->clearClipRegion();
    
    for( int cx=0; cx<8; cx++ ) {
        for( int cy=0; cy<32; cy++ ) {
            // Get the offset of this location in the video RAM
            unsigned offset = 32*cy + (cx ^ 4);
            
            unsigned v = video_ram[offset];
            unsigned c = video_ram[offset+0x800];
            
            unsigned m = 0;
            
            if( (c & 0x40) == 0 ) m |= opFlipX;
            if( (c & 0x80) != 0 ) m |= opFlipY;
            
            TBltAdd * bg_blitter = (m & opFlipX) ? &bg_blitter_r : &bg_blitter_f;
            
            int x = cx*8 + 28*8;
            int y = cy*8 - 16;
            
            c &= 0x3F;
            
            screen()->bits()->copy( x, y, char_data_, 0, 8*v, 8, 8, m, bg_blitter->color(c*4) );
        }
    }

    
    // ...the dots on the radar...
    unsigned char * radarx = video_ram + 0x20;
    unsigned char * radary = radarx + 0x800;
    unsigned char * radarattr = main_board_->radar_ram_;
    
    TBltAddSrcTrans dot_blitter(0, 3);
    
	for(int o = 0x14; o < 0x20; o++) {
		int x = radarx[o] + ((~radarattr[o & 0x0F] & 0x01) << 8);
		int y = 253 - 16 - radary[o];
        int v = ((radarattr[o & 0x0F] & 0x0F) >> 1) ^ 0x07;
        
        screen()->bits()->copy( x, y, dots_data_, 0, 4*v, 4, 4, opAdd, dot_blitter.color(16) );
	}
    
    // ...then the sprites...
    screen()->bits()->setClipRegion( 0, 0, 224, 224 );
    
    TBltAddXlatSrcTrans         sprite_blitter_f(0, 0, palette_xlat_);
    TBltAddXlatSrcTransReverse  sprite_blitter_r(0, 0, palette_xlat_);
    
    for (int o = 0x1E; o >= 0x14; o -= 2) {
        int x = video_ram[o + 1] + ((video_ram[0x800 + o + 1] & 0x80) << 1) - 4;
        int y = 241 - 16 - video_ram[0x800 + o] - 1;
        int c = video_ram[0x800 + o + 1] & 0x3F;
        int v = video_ram[o] >> 2;
        
        unsigned mode = 0;
        
        if( video_ram[o] & 1 ) mode |= opFlipX;
        if( video_ram[o] & 2 ) mode |= opFlipY;
        
        TBltAdd * blitter = mode & opFlipX ? (TBltAddXlatSrcTrans*) &sprite_blitter_r : &sprite_blitter_f;
        
        screen()->bits()->copy( x, y, sprite_data_, 0, 16*v, 16, 16, mode, blitter->color( c*4 ) );
    }
    
    // ...and finally the foreground characters
    for( int cx=0; cx<32; cx++ ) {
        for( int cy=0; cy<32; cy++ ) {
            // Get the offset of this location in the video RAM
            unsigned offset = 32*cy + cx;
            
            unsigned v = video_ram[offset+0x400];
            unsigned c = video_ram[offset+0xC00];
            
            if( (c & 0x20) == 0)  continue;
            
            unsigned m = 0;
            
            if( (c & 0x40) == 0 ) m |= opFlipX;
            if( (c & 0x80) != 0 ) m |= opFlipY;
            
            TBltAdd * fg_blitter = (m & opFlipX) ? &bg_blitter_r : &bg_blitter_f;
            
            int x = cx*8;
            int y = cy*8 - 16;
            
            c &= 0x3F;
            
            screen()->bits()->copy( x, y, char_data_, 0, 8*v, 8, 8, m, fg_blitter->color(c*4) );
        }
    }
    
    return screen();
}

static const TDecodeCharInfo8x8 charLayout =
{
    2,
    { 0, 4 },
	{  8*8+3, 8*8+2, 8*8+1, 8*8+0, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 }
};

static const TDecodeCharInfo spriteLayout =
{
    16, 16,
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 24*8+0, 24*8+1, 24*8+2, 24*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
    { 39*8, 38*8, 37*8, 36*8, 35*8, 34*8, 33*8, 32*8, 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 }
};

static const TDecodeCharInfo dotLayout =
{
    4, 4,
	2,
	{ 0, 1 },
	{ 2*8, 3*8, 0*8, 1*8 },
    { 3*32, 2*32, 1*32, 0*32 },
};

// Update the internal tables after a ROM has changed
void RallyX::onVideoROMsChanged()
{
    // Refresh palette
    for( int i=0; i<32; i++ ) {
        palette()->setColor( i, TPalette::decodeByte(palette_prom_[i]) );
    }

    for( int i=0; i<256; i++ ) {
        palette_xlat_[i] = palette_lookup_prom_[i] & 0x0F;
    }
    
    // Characters (256 8x8x2 characters)
    decodeCharSet8x8( (unsigned char *) char_data_.data(), charLayout, video_rom_, 256, 16 );
    
    // Sprites (512 16x16x4 sprites)
    decodeCharSet( (unsigned char *) sprite_data_.data(), spriteLayout, video_rom_, 64, 16*16*2 );

    // Dots in the radar screen (8 4x4x2 sprites)
    decodeCharSet( (unsigned char *) dots_data_.data(), dotLayout, dots_prom_, 8, 4*4*8 );
}

NewRallyX::NewRallyX( RallyXMainBoard * board ) :
    RallyX( board )
{
    registerDriver( NewRallyXInfo );
}

bool NewRallyX::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    switch( id ) {
    case Rx1B:
        memcpy( main_board_->rom_+0x0000, buf,       0x800 );
        memcpy( main_board_->rom_+0x1000, buf+0x800, 0x800 );
        return true;
    case Rx1E:
        memcpy( main_board_->rom_+0x0800, buf,       0x800 );
        memcpy( main_board_->rom_+0x1800, buf+0x800, 0x800 );
        return true;
    case Rx1H:
        memcpy( main_board_->rom_+0x2000, buf,       0x800 );
        memcpy( main_board_->rom_+0x3000, buf+0x800, 0x800 );
        return true;
    case Rx1K:
        memcpy( main_board_->rom_+0x2800, buf,       0x800 );
        memcpy( main_board_->rom_+0x3800, buf+0x800, 0x800 );
        return true;
    }
    
    return RallyX::setResourceFile( id, buf, len );
}

bool NewRallyX::initialize( TMachineDriverInfo * info )
{
    RallyX::initialize( info );

    resourceHandler()->replace( Rx1B, "nrx_prg1.1d", 0, 0 );
    resourceHandler()->replace( Rx1E, "nrx_prg2.1e", 0, 0 );
    resourceHandler()->replace( Rx1H, "nrx_prg3.1k", 0, 0 );
    resourceHandler()->replace( Rx1K, "nrx_prg4.1l", 0, 0 );
    
    resourceHandler()->remove( Rx8E );

    resourceHandler()->add( Rx8E, "nrx_chg1.8e", 0x800, efVideoROM, video_rom_ );
    resourceHandler()->add( Rx8D, "nrx_chg2.8d", 0x800, efVideoROM, video_rom_+0x800 );
    
    resourceHandler()->addToMachineDriverInfo( info );
    
    return true;
}
