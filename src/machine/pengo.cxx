/*
    Pengo arcade machine emulator

    Copyright (c) 2006 Alessandro Scotti
*/
#include "pengo.h"

// Output flip-flops
enum {
    FlipScreen          = 0x01,
    InterruptEnabled    = 0x08,
    SoundEnabled        = 0x10,
    CoinLockout         = 0x20,
    CoinMeter1          = 0x40,
    CoinMeter2          = 0x80
};

// Hardware info
enum {
    ScreenWidth         = 224,
    ScreenHeight        = 288,
    ScreenColors        = 256,
    VideoFrequency      = 60,
    CpuClock            = 3072000,
    SoundClock          = 96000,    // CPU clock divided by 32
    CpuCyclesPerFrame   = CpuClock / VideoFrequency
};

enum { 
    Ep8, Ep7, Ep15, Ep14, Ep21, Ep20, Ep32, Ep31, Ep92, Ep105, Pr78, Pr88, Pr51, Pr70,
    OptCoinA, OptCoinB, OptLives, OptBonus, OptDifficulty, OptDemoSounds, OptRackTest, OptServiceMode, OptCabinet
};

// Machine info
static TMachineInfo PengoInfo = { 
    "pengo", "Pengo (set 1)", S_Sega, 1982, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo Pengo2uInfo = { 
    "pengo2u", "Pengo (set 2 not encrypted)", S_Sega, 1982, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TGameRegistrationHandler reg1( &Pengo2uInfo, Pengo::createInstance );

bool Pengo::initialize( TMachineDriverInfo * info )
{
    // Declare resources
    resourceHandler()->add( Pr51,  "pr1635.51",   0x100, efSoundPROM,   0 );
    
    resourceHandler()->add( Pr88,  "pr1634.88",   0x400, efColorPROM,   color_prom_ );
    
    resourceHandler()->add( Pr78,  "pr1633.78",   0x20, efPalettePROM, palette_prom_ );
    
    resourceHandler()->add( Ep8,   "ep1689c.8",   0x1000, efROM,         main_board_->ram_+0x0000 );
    resourceHandler()->add( Ep7,   "ep1690b.7",   0x1000, efROM,         main_board_->ram_+0x1000 );
    resourceHandler()->add( Ep15, "ep1691b.15",   0x1000, efROM,         main_board_->ram_+0x2000 );
    resourceHandler()->add( Ep14, "ep1692b.14",   0x1000, efROM,         main_board_->ram_+0x3000 );
    resourceHandler()->add( Ep21, "ep1693b.21",   0x1000, efROM,         main_board_->ram_+0x4000 );
    resourceHandler()->add( Ep20, "ep1694b.20",   0x1000, efROM,         main_board_->ram_+0x5000 );
    resourceHandler()->add( Ep32, "ep5118b.32",   0x1000, efROM,         main_board_->ram_+0x6000 );
    resourceHandler()->add( Ep31, "ep5119c.31",   0x1000, efROM,         main_board_->ram_+0x7000 );
    
    resourceHandler()->add( Ep92,  "ep1640.92",   0x2000, efVideoROM,    video_rom_+0x0000 );
    resourceHandler()->add( Ep105,"ep1695.105",   0x2000, efVideoROM,    video_rom_+0x2000 );

    // Pengo set 2 not encrypted
    resourceHandler()->replace( Ep8,   "pengo.u8",   0x00000000 );
    resourceHandler()->replace( Ep7,   "pengo.u7",   0x00000000 );
    resourceHandler()->replace( Ep15, "pengo.u15",   0x00000000 );
    resourceHandler()->replace( Ep14, "pengo.u14",   0x00000000 );
    resourceHandler()->replace( Ep21, "ep5124.21",   0x00000000 );
    resourceHandler()->replace( Ep20, "pengo.u20",   0x00000000 );
    resourceHandler()->replace( Ep32, "ep5126.32",   0x00000000 );
    resourceHandler()->replace( Ep31, "pengo.u31",   0x00000000 );
    
    resourceHandler()->addToMachineDriverInfo( info );

    // Setup user interface
    TUserInterface * ui = info->userInterface();
    
    TUiOptionGroup * group = ui->addGroup( S_DSW_1, dtDipSwitch );

    TUiOption * option = group->add( OptBonus, "Bonus PENGO at", 0, 0x01 );
    option->add( "30000", 0x00 );
    option->add( "50000", 0x01 );

    option = group->add( OptDemoSounds, "Attract sounds", 1, 0x02 );
    option->add( S_On, 0x00 );
    option->add( S_Off, 0x02 );

    option = group->add( OptCabinet, S_Cabinet, 0, 0x04 );
    option->add( S_Upright, 0x00 );
    option->add( S_Cocktail, 0x04 );

    option = group->add( OptLives, S_Lives, 1, 0x18 );
    option->add( "2", 0x18 );
    option->add( "3", 0x10 );
    option->add( "4", 0x08 );
    option->add( "5", 0x00 );

    option = group->add( OptRackTest, "Rack test", 0, 0x20 );
    option->add( "Off (normal play)", 0x20 );
    option->add( "On (continuous play)", 0x00 );

    option = group->add( OptDifficulty, "Difficulty", 1, 0xC0 );
    option->add( S_Easy, 0xC0 );
    option->add( "Medium", 0x80 );
    option->add( S_Hard, 0x40 );
    option->add( S_Hardest, 0x00 );
    
    group = ui->addGroup( S_DSW_2, dtDipSwitch );
    
    option = group->add( OptCoinA, "Coin A", 12, 0x0F );
    option->add( S_4Coin1Play, 0x00 );
    option->add( S_1Coin6Play, 0x01 );
    option->add( S_1Coin2Play, 0x02 );
    option->add( "1 Coin/1 Play (OOCC)", 0x03 );
    option->add( S_2Coin1Play, 0x04 );
    option->add( "2 Coin/1 Play (OCOC)", 0x05 );
    option->add( S_1Coin4Play, 0x06 );
    option->add( "1 Coin/2 Play (OOOC)", 0x07 );
    option->add( S_3Coin1Play, 0x08 );
    option->add( "2 Coin/1 Play (OCCO)", 0x09 );
    option->add( S_1Coin3Play, 0x0A );
    option->add( "1 Coin/1 Play (OOCO)", 0x0B );
    option->add( S_1Coin1Play, 0x0C );
    option->add( "1 Coin/1 Play (OCOO)", 0x0D );
    option->add( S_1Coin5Play, 0x0E );
    option->add( "1 Coin/2 Play (OOOO)", 0x0F );

    option = group->add( OptCoinB, "Coin B", 12, 0xF0 );
    option->add( S_4Coin1Play, 0x00 );
    option->add( S_1Coin6Play, 0x10 );
    option->add( S_1Coin2Play, 0x20 );
    option->add( "1 Coin/1 Play (OOCC)", 0x30 );
    option->add( S_2Coin1Play, 0x40 );
    option->add( "2 Coin/1 Play (OCOC)", 0x50 );
    option->add( S_1Coin4Play, 0x60 );
    option->add( "1 Coin/2 Play (OOOC)", 0x70 );
    option->add( S_3Coin1Play, 0x80 );
    option->add( "2 Coin/1 Play (OCCO)", 0x90 );
    option->add( S_1Coin3Play, 0xA0 );
    option->add( "1 Coin/1 Play (OOCO)", 0xB0 );
    option->add( S_1Coin1Play, 0xC0 );
    option->add( "1 Coin/1 Play (OCOO)", 0xD0 );
    option->add( S_1Coin5Play, 0xE0 );
    option->add( "1 Coin/2 Play (OOOO)", 0xF0 );
    
    group = ui->addGroup( "Service", dtDipSwitch );
    
    option = group->add( OptServiceMode, "Service mode", 0, 0x10 );
    option->add( S_Normal, 0x10 );
    option->add( S_Test, 0x00 );

    optionHandler()->add( &main_board_->dsw0_, ui, OptBonus, OptDemoSounds, OptLives, OptRackTest, OptDifficulty );
    optionHandler()->add( &main_board_->dsw0_, ui, OptCabinet );
    optionHandler()->add( &main_board_->dsw1_, ui, OptCoinA, OptCoinB );
    optionHandler()->add( &main_board_->port1_, ui, OptServiceMode );

    // Ask notification so internal variables are same as GUI
    return true;
}

bool Pengo::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    // Need to refresh some internal tables if the video/color ROMs change
    refresh_roms_ |= (id <= Ep8) || (id == Ep105);

    if( id == Pr51 ) {
        main_board_->setSoundPROM( buf );
        return true;
    }

    return 0 == resourceHandler()->handle( id, buf, len );
}

Pengo::Pengo( PengoBoard * board ) :
    main_board_( board ),
    char_data_( 8, 8*256*2 ),
    sprite_data_( 16, 16*64*2 )
{
    frame_counter_ = 0;
    flip_monitor_ = false;
    refresh_roms_ = false;

    createScreen( ScreenWidth, ScreenHeight, ScreenColors );

    setJoystickHandler( 0, new TJoystickToPortHandler(idJoyP1Joystick1, ptInverted, jm4Way, &board->port0_, 0x02010804) );
    setJoystickHandler( 1, new TJoystickToPortHandler(idJoyP2Joystick1, ptInverted, jm4Way, &board->port1_, 0x02010804) );

    eventHandler()->add( idCoinSlot1,        ptInverted, &board->port0_, 0x10 );
    eventHandler()->add( idCoinSlot2,        ptInverted, &board->port0_, 0x20 );
    eventHandler()->add( idKeyStartPlayer1,  ptInverted, &board->port1_, 0x20 );
    eventHandler()->add( idKeyStartPlayer2,  ptInverted, &board->port1_, 0x40 );
    eventHandler()->add( idKeyP1Action1,     ptInverted, &board->port0_, 0x80 );
    eventHandler()->add( idKeyP2Action1,     ptInverted, &board->port1_, 0x80 );

    registerDriver( PengoInfo ); // Register also set 1 so we can get video and sound PROMs
    registerDriver( Pengo2uInfo );
}

Pengo::~Pengo()
{
    delete main_board_;
}

void Pengo::reset()
{
    main_board_->reset();

    frame_counter_ = 0;
}

void Pengo::run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate )
{
    // Update if ROM changed since last frame
    if( refresh_roms_ ) {
        onVideoROMsChanged();
        refresh_roms_ = false;
    }

    // Update the watchdog timer
    main_board_->watchdog_.tick( this );

    // Run the game for one frame
    main_board_->run();

    // Render the video every other frame
    frame_counter_++;

    if( frame_counter_ & 1 ) {
        frame->setVideo( renderVideo(), flip_monitor_ );
    }

    // Play the sound if enabled
    if( main_board_->output_devices_ & SoundEnabled ) {
        TMixerBuffer * mixer_buffer = frame->getMixer()->getBuffer( chMono, samplesPerFrame, 3 );
        int * data_buffer = mixer_buffer->data();

        // Let the chip play the sound
        main_board_->sound_chip_.setSamplingRate( samplingRate );
        main_board_->sound_chip_.playSound( data_buffer, samplesPerFrame );
    }
}

void Pengo::decodeCharByte( unsigned char b, unsigned char * charbuf, int charx, int chary, int charwidth )
{
    for( int i=3; i>=0; i-- ) {
        charbuf[charx+(chary+i)*charwidth] = (b & 1) | ((b >> 3) & 2);
        b >>= 1;
    }
}

void Pengo::decodeCharLine( unsigned char * src, unsigned char * charbuf, int charx, int chary, int charwidth )
{
    for( int x=7; x>=0; x-- ) {
        decodeCharByte( *src++, charbuf, x+charx, chary, charwidth );
    }
}

// Update the internal tables after a ROM has changed
void Pengo::onVideoROMsChanged()
{
    int i;
    
    for( int bank=0; bank<2; bank++ ) {
        // Decode character set
        for( i=0; i<256; i++ ) {
            unsigned char * src = video_rom_ + 16*i + 0x2000*bank;
            unsigned char * dst = (unsigned char *)char_data_.data() + 64*i + 64*256*bank;
            
            decodeCharLine( src,   dst, 0, 4, 8 );
            decodeCharLine( src+8, dst, 0, 0, 8 );
        }
        
        // Decode sprite set
        for( i=0; i<64; i++ ) {
            unsigned char * src = video_rom_ + 0x1000 + i*64 + 0x2000*bank;
            unsigned char * dst = (unsigned char *)sprite_data_.data() + 256*i + 64*256*bank;
            
            decodeCharLine( src   , dst, 8, 12, 16 );
            decodeCharLine( src+ 8, dst, 8,  0, 16 );
            decodeCharLine( src+16, dst, 8,  4, 16 );
            decodeCharLine( src+24, dst, 8,  8, 16 );
            decodeCharLine( src+32, dst, 0, 12, 16 );
            decodeCharLine( src+40, dst, 0,  0, 16 );
            decodeCharLine( src+48, dst, 0,  4, 16 );
            decodeCharLine( src+56, dst, 0,  8, 16 );
        }
    }
}

TBitmapIndexed * Pengo::renderVideo()
{
    // Decode palette
    for( int j=0; j<32; j++ ) {
        palette()->setColor( j, TPalette::decodeByte(palette_prom_[(j + main_board_->palette_bank_*16) & 0x1F]) );
    }

    // Draw the background first...
    unsigned char * video = main_board_->ram_+0x8000;
    unsigned char * color = main_board_->ram_+0x8400;
    unsigned char * color_prom = color_prom_ + 128*main_board_->color_bank_;
    
    TBltAddXlat         char_blitter_f(0,color_prom);
    TBltAddXlatReverse  char_blitter_r(0,color_prom);
    TBltAdd * blitter;
    unsigned mode = opAdd;
    unsigned bank = main_board_->char_bank_;

    if( main_board_->output_devices_ & FlipScreen ) {
        // Change mode and switch to the reverse blitter
        mode |= opFlipX | opFlipY;
        blitter = &char_blitter_r;
    }
    else {
        blitter = &char_blitter_f;
    }
    
    for( int y=0; y<36; y++ ) {
        for( int x=0; x<28; x++ ) {
            // Get the offset of this location in the video RAM
            unsigned offset;

            if( y < 2 ) {
                offset = 0x3DD - x + 0x20*(y & 1);
            }
            else if( y >= 34 ) {
                offset = 0x01D - x + 0x20*(y & 1);
            }
            else {
                offset = 0x3A0 + y - 2 - 0x20*x;
            }

            int dx = (mode & opFlipX) ? 27-x : x;
            int dy = (mode & opFlipY) ? 35-y : y;
            
            screen()->bits()->copy( dx*8, dy*8, char_data_, 0, bank*8*256+8*video[offset], 8, 8, mode, blitter->color( 4*(0x1F & color[offset]) ) );
        }
    }
    
    // ...then add the sprites
    screen()->bits()->setClipRegion( 0, 16, 224, 272 );
    
    for( int i=6; i>=1; i-- ) {
        unsigned char * data = main_board_->ram_ + 0x8FF0 + i*2;
        unsigned char color = data[1];
        unsigned index = data[0] >> 2;
        int x = 240 - (int)main_board_->sprite_coords_[i*2+0] - 1;
        int y = 272 - (int)main_board_->sprite_coords_[i*2+1];

        if( main_board_->output_devices_ & FlipScreen ) x--; // Adjust X coordinate when screen is flipped
        
        TBltAddXlatDstTrans         sprite_blitter_f(0, 0, color_prom);
        TBltAddXlatDstTransReverse  sprite_blitter_r(0, 0, color_prom);
        TBltAdd * blitter;

        unsigned mode = opAdd | ((data[0] & 0x01) ? opFlipY : 0);

        if( data[0] & 0x02 ) {
            mode |= opFlipX;
            blitter = &sprite_blitter_r;
        }
        else {
            blitter = &sprite_blitter_f;
        }

        screen()->bits()->copy( x, y, sprite_data_, 0, bank*16*64+16*index, 16, 16, mode, blitter->color( 4*(color & 0x3F) ) );
    }

    screen()->bits()->clearClipRegion();

    return screen();
}

PengoBoard::PengoBoard() : 
    sound_chip_( SoundClock ),
    watchdog_( 10 ) // Called about 3 times per frame, but may skip whole frames sometimes (e.g. initialization)
{
    // Initialize the CPU and the RAM
    cpu_ = new Z80( *this );
    memset( ram_, 0xFF, sizeof(ram_) );

    // Initialize parameters
    port0_ = 0xFF;
    port1_ = 0xFF;
    dsw0_ = 0x00;
    dsw1_ = 0x00;
    coin_counter1_ = 0;
    coin_counter2_ = 0;
    char_bank_ = 0;
    palette_bank_ = 0;
    color_bank_ = 0;
    coin1_timer_ = 0;
    coin2_timer_ = 0;

    // Reset the machine
    reset();
}

PengoBoard::~PengoBoard()
{
    delete cpu_;
}

void PengoBoard::reset()
{
    cpu_->reset();

    output_devices_ = 0;

    memset( ram_+0x8000, 0, 0x1000 );
    memset( sprite_coords_, 0, sizeof(sprite_coords_) );
}

void PengoBoard::run()
{
    // The game resets if the "coin inserted" signal is enabled for too long, make sure it gets cleared after a while
    if( coin1_timer_ > 0 ) {
        if( --coin1_timer_ == 0 ) {
            port0_ |= 0x10;
        }
    }
    
    if( coin2_timer_ > 0 ) {
        if( --coin2_timer_ == 0 ) {
            port0_ |= 0x20;
        }
    }
    
    // Run
    cpu_->run( CpuCyclesPerFrame );

    // If interrupts are enabled, force a CPU interrupt
    if( output_devices_ & InterruptEnabled ) {
        cpu_->interrupt( 0 );
    }
}

unsigned char PengoBoard::readByte( unsigned addr ) 
{
    addr &= 0xFFFF;

    if( addr < sizeof(ram_) )
        return ram_[addr];
    
    // Address is not in RAM, check to see if it's a memory mapped register
    switch( addr & 0xFFC0 ) {
    case 0x9000: 
        return dsw1_;
    case 0x9040: 
        return dsw0_;
    case 0x9080:
        return port1_;
    case 0x90C0:
        if( (port0_ & 0x10) == 0 && coin1_timer_ == 0 ) coin1_timer_ = 3;
        if( (port0_ & 0x20) == 0 && coin2_timer_ == 0 ) coin2_timer_ = 3;
        return port0_;
    }

    return 0xFF;
}

void PengoBoard::setOutputFlipFlop( unsigned char bit, unsigned char value )
{
    if( value ) {
        output_devices_ |= bit;
    }
    else {
        output_devices_ &= ~bit;
    }
}

void PengoBoard::writeByte( unsigned addr, unsigned char b )
{
    addr &= 0xFFFF;
    
    if( addr < 0x8000 ) {
        // This is a ROM address, do not write into it!
    }
    else if( addr < 0x9000 ) {
        // RAM (includes video RAM and sprite RAM)
        ram_[addr] = b;
    }
    else if( addr >= 0x9000 && addr < 0x9020 ) {
        // Sound registers
        sound_chip_.setRegister( addr-0x9000, b );
    }
    else if( addr > 0x9020 && addr < 0x9030 ) {
        // Sprite coordinates
        sprite_coords_[ addr-0x9020 ] = b;
    }
    else switch( addr ) {
        // Memory mapped ports
        case 0x9040:
            // Interrupt enable
            setOutputFlipFlop( InterruptEnabled, b & 0x01 );
            break;
        case 0x9041:
            // Sound enable
            setOutputFlipFlop( SoundEnabled, b & 0x01 );
            break;
        case 0x9042:
            // Palette bank selector
            palette_bank_ = b & 1;
            break;
        case 0x9043:
            // Flip screen
            setOutputFlipFlop( FlipScreen, b & 0x01 );
            break;
        case 0x9044:
            // Coin counter #1 (incremented on 0/1 edge)
            if( (output_devices_ & CoinMeter1) == 0 && (b & 0x01) != 0 )
                coin_counter1_++;
            setOutputFlipFlop( CoinMeter1, b & 0x01 );
            break;
        case 0x9045:
            // Coin counter #2 (incremented on 0/1 edge)
            if( (output_devices_ & CoinMeter2) == 0 && (b & 0x01) != 0 )
                coin_counter2_++;
            setOutputFlipFlop( CoinMeter2, b & 0x01 );
            break;
        case 0x9046:
            // Color lookup table bank selector
            color_bank_ = b & 1;
            break;
        case 0x9047:
            // Character/sprite bank selector
            char_bank_ = b & 1;
            break;
        case 0x9070:
            // Watchdog reset
            watchdog_.reset();
            break;
    }
}
