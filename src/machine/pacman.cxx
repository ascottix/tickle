/*
    Pacman arcade machine emulator

    Copyright (c) 1997-2003,2004 Alessandro Scotti
*/
#include "pacman.h"

// Output flip-flops
enum {
    FlipScreen          = 0x01,
    PlayerOneLight      = 0x02,
    PlayerTwoLight      = 0x04,
    InterruptEnabled    = 0x08,
    SoundEnabled        = 0x10,
    CoinLockout         = 0x20,
    CoinMeter           = 0x40,
    AuxBoardEnabled     = 0x80
};

// Hardware info
enum {
    ScreenWidth         = 224,
    ScreenHeight        = 288,
    ScreenColors        = 256,
    VideoFrequency      = 60,
    CpuClock            = 3072000,
    SoundClock          = CpuClock / 32,
    CpuCyclesPerFrame   = CpuClock / VideoFrequency
};

enum { 
    Ef4A, Ef7F, Ef5E, Ef5F, Ef1M, Ef6E, Ef6F, Ef6H, Ef6J, EfU5, EfU6, EfU7,
    OptCoinage, OptLives, OptBonus, OptDifficulty, OptGhostNames,
    OptRackTest, OptRackAdvance, OptCabinet, OptTeleport
};

// Machine info
static TMachineInfo PuckmanInfo = { 
    "puckman", "Puckman", S_Namco, 1980, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo PacmanInfo = { 
    "pacman", "Pacman", S_Midway, 1980, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo MsPacmanInfo = { 
    "mspacman", "Ms. Pacman", S_Midway, 1981, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo PacmanPlusInfo = { 
    "pacplus", "Pacman Plus", S_Midway, 1982, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo MakeTraxInfo = { 
    "maketrax", "Make Trax", S_Williams, 1981, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo CrushRollerInfo = { 
    "crush", "Crush Roller", S_KuralSamno, 1981, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo EyesInfo = { 
    "eyes", "Eyes", S_RockOla, 1982, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo JumpShotInfo = { 
    "jumpshot", "Jump Shot", S_BallyMidway, 1985, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TGameRegistrationHandler reg1( &PacmanInfo, Pacman::createInstance );
static TGameRegistrationHandler reg2( &PuckmanInfo, Puckman::createInstance );
static TGameRegistrationHandler reg3( &PacmanPlusInfo, PacmanPlus::createInstance );
static TGameRegistrationHandler reg4( &MsPacmanInfo, MsPacman::createInstance );
static TGameRegistrationHandler reg5( &MakeTraxInfo, MakeTrax::createInstance );
static TGameRegistrationHandler reg6( &CrushRollerInfo, CrushRoller::createInstance );
static TGameRegistrationHandler reg7( &EyesInfo, Eyes::createInstance );
static TGameRegistrationHandler reg8( &JumpShotInfo, JumpShot::createInstance );

bool Puckman::initialize( TMachineDriverInfo * info )
{
    // Declare resources
    resourceHandler()->add( Ef1M,  "82s126.1m", 0x100, efSoundPROM, 0 );
    
    resourceHandler()->add( Ef4A,  "82s126.4a", 0x100, efColorPROM, color_prom_ );
    
    resourceHandler()->add( Ef7F,  "82s123.7f", 0x20, efPalettePROM, palette_prom_ );
    
    resourceHandler()->add( Ef6E,"namcopac.6e", 0x1000, efROM, main_board_->ram_+0x0000 );
    resourceHandler()->add( Ef6F,"namcopac.6f", 0x1000, efROM, main_board_->ram_+0x1000 );
    resourceHandler()->add( Ef6H,"namcopac.6h", 0x1000, efROM, main_board_->ram_+0x2000 );
    resourceHandler()->add( Ef6J,"namcopac.6j", 0x1000, efROM, main_board_->ram_+0x3000 );
    
    resourceHandler()->add( Ef5E,  "pacman.5e", 0x1000, efVideoROM, video_rom_+0x0000 );
    resourceHandler()->add( Ef5F,  "pacman.5f", 0x1000, efVideoROM, video_rom_+0x1000 );

    resourceHandler()->addToMachineDriverInfo( info );

    // Setup user interface
    TUserInterface * ui = info->userInterface();
    TUiOptionGroup * group = ui->addGroup( S_DSW_1, dtDipSwitch );

    TUiOption * option = group->add( OptCoinage, S_Coin, 1, 0x03 );
    option->add( S_FreePlay, 0x00 );
    option->add( S_1Coin1Play, 0x01 );
    option->add( S_1Coin2Play, 0x02 );
    option->add( S_2Coin1Play, 0x03 );

    option = group->add( OptLives, S_Lives, 2, 0x0C );
    option->add( "1", 0x00 );
    option->add( "2", 0x04 );
    option->add( "3", 0x08 );
    option->add( "5", 0x0C );

    option = group->add( OptBonus, "Bonus life at", 0, 0x30 );
    option->add( "10000", 0x00 );
    option->add( "15000", 0x10 );
    option->add( "20000", 0x20 );
    option->add( "None", 0x30 );

    option = group->add( OptDifficulty, "Difficulty", 0, 0x40 );
    option->add( S_Normal, 0x40 );
    option->add( S_Hard, 0x00 );

    option = group->add( OptGhostNames, "Ghost names", 0, 0x80 );
    option->add( S_Normal, 0x80 );
    option->add( S_Alternate, 0x00 );

    group = ui->addGroup( "Board Settings", dtDipSwitch );

    option = group->add( OptRackTest, "Rack mode", 0, 0x10 );
    option->add( S_Normal, 0x10 );
    option->add( S_Test, 0x00 );

    option = group->add( OptRackAdvance, "Rack advance", 0, 0x10 );
    option->add( S_Off, 0x10 );
    option->add( S_On, 0x00 );

    option = group->add( OptCabinet, S_Cabinet, 0, 0x80 );
    option->add( S_Upright, 0x80 );
    option->add( S_Cocktail, 0x00 );

    optionHandler()->add( &main_board_->dip_switches_, ui, OptCoinage, OptLives, OptBonus, OptDifficulty, OptGhostNames );
    optionHandler()->add( &main_board_->port1_, ui, OptRackAdvance );
    optionHandler()->add( &main_board_->port2_, ui, OptRackTest, OptCabinet );

    // Ask notification so internal variables are same as GUI
    return true;
}

bool Puckman::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    // Need to refresh some internal tables if the video/color ROMs change
    refresh_roms_ |= (id <= Ef5E) || (id == Ef7F);

    if( id == Ef1M ) {
        main_board_->setSoundPROM( buf );
        return true;
    }

    return 0 == resourceHandler()->handle( id, buf, len );
}

bool Pacman::initialize( TMachineDriverInfo * info )
{
    static TResourceHandlerReplaceInfo ri[] = {
        { Ef6E, "pacman.6e", 0x0000000 },
        { Ef6F, "pacman.6f", 0x0000000 },
        { Ef6H, "pacman.6h", 0x0000000 },
        { Ef6J, "pacman.6j", 0x0000000 }
    };

    Puckman::initialize( info );

    resourceHandler()->replace( ri, sizeof(ri) / sizeof(ri[0]) );

    resourceHandler()->addToMachineDriverInfo( info );

    return true;
}

Pacman::Pacman( PacmanBoard * board ) : Puckman( board )
{
    registerDriver( PacmanInfo );
}

Puckman::Puckman( PacmanBoard * board ) :
    main_board_( board ),
    char_data_( 8, 8*256 ),
    sprite_data_( 16, 16*64 )
{
    frame_counter_ = 0;
    flip_monitor_ = false;
    refresh_roms_ = false;

    createScreen( ScreenWidth, ScreenHeight, ScreenColors );

    setJoystickHandler( 0, new TJoystickToPortHandler(idJoyP1Joystick1, ptInverted, jm4Way, &board->port1_, 0x08010402) );
    setJoystickHandler( 1, new TJoystickToPortHandler(idJoyP2Joystick1, ptInverted, jm4Way, &board->port2_, 0x08010402) );

    eventHandler()->add( idCoinSlot1,        ptInverted, &board->port1_, 0x20 );
    eventHandler()->add( idCoinSlot2,        ptInverted, &board->port1_, 0x40 );
    eventHandler()->add( idKeyStartPlayer1,  ptInverted, &board->port2_, 0x20 );
    eventHandler()->add( idKeyStartPlayer2,  ptInverted, &board->port2_, 0x40 );
    eventHandler()->add( idKeyService1,      ptInverted, &board->port1_, 0x80 ); // Add credit
    eventHandler()->add( idSwitchServiceTest,ptInverted, &board->port2_, 0x10 );
    eventHandler()->add( idSwitchService1,   ptInverted, &board->port1_, 0x10 ); // Rack advance

    registerDriver( PuckmanInfo );
}

Puckman::~Puckman()
{
    delete main_board_;
}

void Puckman::reset()
{
    main_board_->reset();

    frame_counter_ = 0;
}


void Puckman::run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate )
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

void Puckman::decodeCharByte( unsigned char b, unsigned char * charbuf, int charx, int chary, int charwidth )
{
    for( int i=3; i>=0; i-- ) {
        charbuf[charx+(chary+i)*charwidth] = (b & 1) | ((b >> 3) & 2);
        b >>= 1;
    }
}

void Puckman::decodeCharLine( unsigned char * src, unsigned char * charbuf, int charx, int chary, int charwidth )
{
    for( int x=7; x>=0; x-- ) {
        decodeCharByte( *src++, charbuf, x+charx, chary, charwidth );
    }
}

// Update the internal tables after a ROM has changed
void Puckman::onVideoROMsChanged()
{
    int i;

    // Decode palette
    for( i=0; i<32; i++ ) {
        palette()->setColor( i, TPalette::decodeByte(palette_prom_[i]) );
    }

    // Decode character set
    for( i=0; i<256; i++ ) {
        unsigned char * src = video_rom_ + 16*i;
        unsigned char * dst = (unsigned char *)char_data_.data() + 64*i;

        decodeCharLine( src,   dst, 0, 4, 8 );
        decodeCharLine( src+8, dst, 0, 0, 8 );
    }

    // Decode sprite set
    for( i=0; i<64; i++ ) {
        unsigned char * src = video_rom_ + 0x1000 + i*64;
        unsigned char * dst = (unsigned char *)sprite_data_.data() + 256*i;

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

TBitmapIndexed * Puckman::renderVideo()
{
    // Draw the background first...
    unsigned char * video = main_board_->ram_+0x4000;
    unsigned char * color = main_board_->ram_+0x4400;

    TBltAddXlat         char_blitter_f(0,color_prom_);
    TBltAddXlatReverse  char_blitter_r(0,color_prom_);
    TBltAdd * blitter;
    unsigned mode = opAdd;

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

            screen()->bits()->copy( dx*8, dy*8, char_data_, 0, 8*video[offset], 8, 8, mode, blitter->color( 4*(0x3F & color[offset]) ) );
        }
    }

    // ...then add the sprites
    screen()->bits()->setClipRegion( 0, 16, 224, 272 );
    
    for( int i=7; i>=0; i-- ) {
        unsigned char * data = main_board_->ram_ + 0x4FF0 + i*2;
        unsigned char color = data[1];
        unsigned index = data[0] >> 2;
        int x = 240 - (int)main_board_->sprite_coords_[i*2+0] - 1;
        int y = 272 - (int)main_board_->sprite_coords_[i*2+1];

        TBltAddXlatDstTrans         sprite_blitter_f(0, 0, color_prom_);
        TBltAddXlatDstTransReverse  sprite_blitter_r(0, 0, color_prom_);
        TBltAdd * blitter;

        unsigned mode = opAdd | ((data[0] & 0x01) ? opFlipY : 0);

        if( data[0] & 0x02 ) {
            mode |= opFlipX;
            blitter = &sprite_blitter_r;
        }
        else {
            blitter = &sprite_blitter_f;
        }

        if( i <= 2 ) {
            // In Pacman the first few sprites must be further offset 
            // to the left to get a correct display (this is apparently
            // related to the video refresh timings)
            x--;
        }

        // Handle wraparound (for Make Trax)
        if( y > 256 ) {
            screen()->bits()->copy( x, y-256, sprite_data_, 0, 16*index, 16, 16, mode, blitter->color( 4*(color & 0x3F) ) );
        }
            
        screen()->bits()->copy( x, y, sprite_data_, 0, 16*index, 16, 16, mode, blitter->color( 4*(color & 0x3F) ) );
    }

    screen()->bits()->clearClipRegion();

    return screen();
}

PacmanBoard::PacmanBoard() : 
    sound_chip_( SoundClock ),
    watchdog_( 10 ) // Called about 3 times per frame, but may skip whole frames sometimes (e.g. initialization)
{
    // Initialize the CPU and the RAM
    cpu_ = new Z80( *this );
    memset( ram_, 0xFF, sizeof(ram_) );

    // Initialize parameters
    port1_ = 0xFF;
    port2_ = 0xFF;
    dip_switches_ = 0xC9;
    coin_counter_ = 0;

    // Reset the machine
    reset();
}

PacmanBoard::~PacmanBoard()
{
    delete cpu_;
}

void PacmanBoard::reset()
{
    cpu_->reset();

    output_devices_ = 0;
    interrupt_vector_ = 0;

    memset( ram_+0x4000, 0, 0x1000 );
    memset( sprite_coords_, 0, sizeof(sprite_coords_) );
}

void PacmanBoard::run()
{
    cpu_->run( CpuCyclesPerFrame );

    // If interrupts are enabled, force a CPU interrupt with the vector
    // set by the program
    if( output_devices_ & InterruptEnabled ) {
        cpu_->interrupt( interrupt_vector_ );
    }
}

unsigned char PacmanBoard::readByte( unsigned addr ) 
{
    addr &= 0xFFFF;

    if( addr < sizeof(ram_) )
        return ram_[addr];

    // Address is not in RAM, check to see if it's a memory mapped register
    switch( addr & 0xFFC0 ) {
    // IN0
    case 0x5000: 
        // bit 0 : up
        // bit 1 : left
        // bit 2 : right
        // bit 3 : down
        // bit 4 : switch: advance to next level
        // bit 5 : coin 1
        // bit 6 : coin 2
        // bit 7 : credit (same as coin but coin counter is not incremented)
        return port1_;
    // IN1
    case 0x5040: 
        // bit 0 : up (2nd player)
        // bit 1 : left (2nd player)
        // bit 2 : right (2nd player)
        // bit 3 : down (2nd player)
        // bit 4 : switch: rack test -> 0x10=off, 0=on
        // bit 5 : start 1
        // bit 6 : start 2
        // bit 7 : cabinet -> 0x80=upright, 0x00=table
        return port2_;
    // DSW1
    case 0x5080:
        // bits 0,1 : coinage -> 0=free play, 1=1 coin/play, 2=1 coin/2 play, 3=2 coin/1 play
        // bits 2,3 : lives -> 0x0=1, 0x4=2, 0x8=3, 0xC=5
        // bits 4,5 : bonus life -> 0=10000, 0x10=15000, 0x20=20000, 0x30=none
        // bit  6   : jumper pad: difficulty -> 0x40=normal, 0=hard
        // bit  7   : jumper pad: ghost name -> 0x80=normal, 0=alternate
        return dip_switches_;
    }

    return 0xFF;
}

void PacmanBoard::setOutputFlipFlop( unsigned char bit, unsigned char value )
{
    if( value ) {
        output_devices_ |= bit;
    }
    else {
        output_devices_ &= ~bit;
    }
}

void PacmanBoard::writeByte( unsigned addr, unsigned char b )
{
    addr &= 0x7FFF;

    if( addr < 0x4000 ) {
        // This is a ROM address, do not write into it!
    }
    else if( addr < 0x5000 ) {
        // RAM (includes video RAM and sprite RAM)
        ram_[addr] = b;
    }
    else if( addr >= 0x5040 && addr < 0x5060 ) {
        // Sound registers
        sound_chip_.setRegister( addr-0x5040, b );
    }
    else if( addr >= 0x5060 && addr < 0x5070 ) {
        // Sprite coordinates
        sprite_coords_[ addr-0x5060 ] = b;
    }
    else switch( addr ) {
        // Memory mapped ports
        case 0x5000:
            // Interrupt enable
            setOutputFlipFlop( InterruptEnabled, b & 0x01 );
            break;
        case 0x5001:
            // Sound enable
            setOutputFlipFlop( SoundEnabled, b & 0x01 );
            break;
        case 0x5002:
            // Nothing on original board, enables aux board on Ms. Pacman (see driver)
            break;
        case 0x5003:
            // Flip screen
            setOutputFlipFlop( FlipScreen, b & 0x01 );
            break;
        case 0x5004:
            // Player 1 start light
            setOutputFlipFlop( PlayerOneLight, b & 0x01 );
            break;
        case 0x5005:
            // Player 2 start light
            setOutputFlipFlop( PlayerTwoLight, b & 0x01 );
            break;
        case 0x5006:
            // Coin lockout: bit 0 is used to enable/disable the coin insert slots 
            // (0=disable). The coin slot is enabled at startup and (temporarily) 
            // disabled when the maximum number of available credits (99) is reached
            setOutputFlipFlop( CoinLockout, b & 0x01 );
            break;
        case 0x5007:
            // Coin meter (coin counter incremented on 0/1 edge)
            if( (output_devices_ & CoinMeter) == 0 && (b & 0x01) != 0 )
                coin_counter_++;
            setOutputFlipFlop( CoinMeter, b & 0x01 );
            break;
        case 0x50c0:
            // Watchdog reset
            watchdog_.reset();
            break;
    }
}

void PacmanBoard::writePort( unsigned addr, unsigned char b )
{
    if( (addr & 0x00FF) == 0 ) {
        // Sets the interrupt vector for the next CPU interrupt
        interrupt_vector_ = b;
    }
}

bool MsPacman::initialize( TMachineDriverInfo * info )
{
    Pacman::initialize( info );

    resourceHandler()->replace( Ef5E, "5e", 0x0000000 );
    resourceHandler()->replace( Ef5F, "5f", 0x0000000 );
    
    resourceHandler()->add( EfU5, "u5",  0x800, efROM, encrypted_rom_+0x2000 );
    resourceHandler()->add( EfU6, "u6", 0x1000, efROM, encrypted_rom_+0x1000 );
    resourceHandler()->add( EfU7, "u7", 0x1000, efROM, encrypted_rom_+0x0000 );

    resourceHandler()->addToMachineDriverInfo( info );

    // Setup user interface
    TUserInterface * ui = info->userInterface();

    ui->removeOption( OptGhostNames );

    return true;
}

MsPacman::MsPacman( MsPacmanBoard * board ) :
    Pacman( board )
{
    main_board_ = board;
    main_board_->aux_board_enabled_ = false;
    memset( main_board_->rom_aux_, 0, sizeof(main_board_->rom_aux_) );
    memset( encrypted_rom_, 0, sizeof(encrypted_rom_) );
    refresh_roms_ = true;

    registerDriver( MsPacmanInfo );
}

bool MsPacman::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    // Need to refresh some internal tables if the aux or basic ROMs change
    refresh_roms_ |= (id >= EfU5) || ((id >= Ef6E) && (id <= Ef6H));

    return Pacman::setResourceFile( id, buf, len );
}

void MsPacman::run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate )
{
    if( refresh_roms_ ) {
        main_board_->onROMsChanged( encrypted_rom_ );
        refresh_roms_ = false;
    }

    Pacman::run( frame, samplesPerFrame, samplingRate );
}

/*
    The code to decrypt the Ms. Pacman aux ROMs derives from the
    MAME machine driver (mspacman.c) written by David Widel.
*/
static unsigned char decryptd( unsigned char e )
{
    return ((e & 0xC0) >> 3) | ((e & 0x10) << 2) | ((e & 0x0E) >> 1) | ((e & 0x01) << 7) | (e & 0x20);
}

static unsigned int decrypta1( unsigned int e )
{
    return (e & 0x807) | ((e & 0x400) >> 7) | ((e & 0x200) >> 2) | ((e & 0x080) << 3) | ((e & 0x040) << 2) | ((e & 0x138) << 1);
}

static unsigned int decrypta2( unsigned int e )
{
	return (e & 0x807) | ((e & 0x040) << 4) | ((e & 0x100) >> 3) | ((e & 0x080) << 2) | ((e & 0x600) >> 2) | ((e & 0x028) << 1) | ((e & 0x010) >> 1);
}

unsigned char MsPacmanBoard::readByte( unsigned addr )
{
    addr &= 0xFFFF;

    if( addr == 0 ) {
        aux_board_enabled_ = false;
    }

    if( aux_board_enabled_ ) {
        if( addr <= 0x4000 ) {
            return rom_aux_[addr];
        }
        else if( addr >= 0x8000 ) {
            if( addr < 0x8800 ) {
                return rom_aux_[addr-0x8000+0x6000]; // U5
            }
            else if( addr < 0xA000 ) {
                return rom_aux_[(addr & 0xFFF)+0x5000]; // U6
            }
        }
    }
    else if( addr < sizeof(ram_) ) {
        // Parent's readByte() handles this case too, however this saves a virtual 
        // function call in the likely case that the read location is in the Pacman ROM/RAM
        return ram_[addr];
    }

    return PacmanBoard::readByte( addr );
}

void MsPacmanBoard::writeByte( unsigned addr, unsigned char b )
{
    if( addr == 0x5002 ) {
        // Enable the aux board if writing to port 0x5002
        aux_board_enabled_ = true;
    }

    PacmanBoard::writeByte( addr, b );
}

void MsPacmanBoard::onROMsChanged( unsigned char * encrypted_rom )
{
    unsigned i;

    // Decrypt aux ROMs
	for( i=0; i<0x1000; i++ ) {
		rom_aux_[decrypta1(i)+0x4000] = decryptd(encrypted_rom[i+0x0000]); // U7
		rom_aux_[decrypta1(i)+0x5000] = decryptd(encrypted_rom[i+0x1000]); // U6
	}

	for( i=0; i<0x0800; i++ ) {
		rom_aux_[decrypta2(i)+0x6000] = decryptd(encrypted_rom[i+0x2000]); // U5
	}

    // Copy original ROM, but replace 6J with U7
    memcpy( rom_aux_+0x0000, ram_+0x0000, 0x1000 );
    memcpy( rom_aux_+0x1000, ram_+0x1000, 0x1000 );
    memcpy( rom_aux_+0x2000, ram_+0x2000, 0x1000 );
    memcpy( rom_aux_+0x3000, rom_aux_+0x4000, 0x1000 ); // U7

    // Apply ROM patches (from scattered U5 locations)
	for( i=0; i<8; i++ ) {
        rom_aux_[0x0410+i] = rom_aux_[0x6008+i];
        rom_aux_[0x08E0+i] = rom_aux_[0x61D8+i];
        rom_aux_[0x0A30+i] = rom_aux_[0x6118+i];
        rom_aux_[0x0BD0+i] = rom_aux_[0x60D8+i];
        rom_aux_[0x0C20+i] = rom_aux_[0x6120+i];
        rom_aux_[0x0E58+i] = rom_aux_[0x6168+i];
        rom_aux_[0x0EA8+i] = rom_aux_[0x6198+i];

        rom_aux_[0x1000+i] = rom_aux_[0x6020+i];
        rom_aux_[0x1008+i] = rom_aux_[0x6010+i];
        rom_aux_[0x1288+i] = rom_aux_[0x6098+i];
        rom_aux_[0x1348+i] = rom_aux_[0x6048+i];
        rom_aux_[0x1688+i] = rom_aux_[0x6088+i];
        rom_aux_[0x16B0+i] = rom_aux_[0x6188+i];
        rom_aux_[0x16D8+i] = rom_aux_[0x60C8+i];
        rom_aux_[0x16F8+i] = rom_aux_[0x61C8+i];
        rom_aux_[0x19A8+i] = rom_aux_[0x60A8+i];
        rom_aux_[0x19B8+i] = rom_aux_[0x61A8+i];

        rom_aux_[0x2060+i] = rom_aux_[0x6148+i];
        rom_aux_[0x2108+i] = rom_aux_[0x6018+i];
        rom_aux_[0x21A0+i] = rom_aux_[0x61A0+i];
        rom_aux_[0x2298+i] = rom_aux_[0x60A0+i];
        rom_aux_[0x23E0+i] = rom_aux_[0x60E8+i];
        rom_aux_[0x2418+i] = rom_aux_[0x6000+i];
        rom_aux_[0x2448+i] = rom_aux_[0x6058+i];
        rom_aux_[0x2470+i] = rom_aux_[0x6140+i];
        rom_aux_[0x2488+i] = rom_aux_[0x6080+i];
        rom_aux_[0x24B0+i] = rom_aux_[0x6180+i];
        rom_aux_[0x24D8+i] = rom_aux_[0x60C0+i];
        rom_aux_[0x24F8+i] = rom_aux_[0x61C0+i];
        rom_aux_[0x2748+i] = rom_aux_[0x6050+i];
        rom_aux_[0x2780+i] = rom_aux_[0x6090+i];
        rom_aux_[0x27B8+i] = rom_aux_[0x6190+i];
        rom_aux_[0x2800+i] = rom_aux_[0x6028+i];
        rom_aux_[0x2B20+i] = rom_aux_[0x6100+i];
        rom_aux_[0x2B30+i] = rom_aux_[0x6110+i];
        rom_aux_[0x2BF0+i] = rom_aux_[0x61D0+i];
        rom_aux_[0x2CC0+i] = rom_aux_[0x60D0+i];
        rom_aux_[0x2CD8+i] = rom_aux_[0x60E0+i];
        rom_aux_[0x2CF0+i] = rom_aux_[0x61E0+i];
        rom_aux_[0x2D60+i] = rom_aux_[0x6160+i];
	}
}

/*
    The code to decrypt the Pacman Plus ROMs derives from the
    MAME machine driver (pacplus.c).
*/
static unsigned char pp_decrypt( unsigned addr, unsigned char e )
{
	static const unsigned char swap_xor_table[6][9] =
	{
		{ 7,6,5,4,3,2,1,0, 0x00 },
		{ 7,6,5,4,3,2,1,0, 0x28 },
		{ 6,1,3,2,5,7,0,4, 0x96 },
		{ 6,1,5,2,3,7,0,4, 0xbe },
		{ 0,3,7,6,4,2,1,5, 0xd5 },
		{ 0,3,4,6,7,2,1,5, 0xdd }
	};

	static const int picktable[32] =
	{
		0,2,4,2,4,0,4,2,2,0,2,2,4,0,4,2,
		2,2,4,0,4,2,4,0,0,4,0,4,4,2,4,2
	};

	// Pick method from bits 0,2,5,7,9 of the address
	unsigned int method = picktable[
		((addr & 0x001)     ) |
		((addr & 0x004) >> 1) |
		((addr & 0x020) >> 3) |
		((addr & 0x080) >> 4) |
		((addr & 0x200) >> 5)];

	// Switch method if bit 11 of the address is set
	if ((addr & 0x800) == 0x800)
		method ^= 1;

	const unsigned char * tbl = swap_xor_table[method];

    // Swap the bits according to the table
    unsigned char result = 0;

    for( int i=7; i>=0; i-- ) {
        if( e & (1 << *tbl++) ) {
            result |= (1 << i);
        }
    }

    return result ^ *tbl;
}

bool PacmanPlus::initialize( TMachineDriverInfo * info )
{
    static TResourceHandlerReplaceInfo ri[] = {
        { Ef4A, "pacplus.4a", 0x0000000 },
        { Ef7F, "pacplus.7f", 0x0000000 },
        { Ef6E, "pacplus.6e", 0x0000000 },
        { Ef6F, "pacplus.6f", 0x0000000 },
        { Ef6H, "pacplus.6h", 0x0000000 },
        { Ef6J, "pacplus.6j", 0x0000000 },
        { Ef5E, "pacplus.5e", 0x0000000 },
        { Ef5F, "pacplus.5f", 0x0000000 }
    };

    Puckman::initialize( info );

    resourceHandler()->replace( ri, sizeof(ri) / sizeof(ri[0]) );

    resourceHandler()->addToMachineDriverInfo( info );

    return true;
}

PacmanPlus::PacmanPlus() : Puckman( new PacmanBoard )
{
    registerDriver( PacmanPlusInfo );
}

bool PacmanPlus::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    unsigned char x_buf[0x1000];

    if( (id >= Ef6E) && (id <= Ef6J) ) {
        // Decrypt ROM
        assert( len <= sizeof(x_buf) );

        for( unsigned i=0; i<len; i++ ) {
            x_buf[i] = pp_decrypt( i, buf[i] );
	    }

        buf = x_buf;
    }

    return Puckman::setResourceFile( id, buf, len );
}

// In MakeTrax, must match values stored in ROM at 0x0EBD
// (one byte here corresponds to two ROM bytes)
unsigned char prot[30] = {
    0x1F, 0xFF, 0x2F, 0x6F, 0xCF, 0x4F, 0x0F, 0xFF, 0x0F, 0x4F,
    0x1C, 0xFC, 0x2C, 0x6C, 0xCC, 0x4C, 0x0C, 0xFC, 0x0C, 0x4C,
    0x11, 0xF1, 0x21, 0x61, 0xC1, 0x41, 0x01, 0xF1, 0x01, 0x41 };

unsigned prot_index_hi = 0;
unsigned prot_index_lo = 0;

unsigned char CrushRollerBoard::readByte( unsigned addr )
{
    addr &= 0xFFFF;

    if( addr < sizeof(ram_) ) {
        return ram_[addr];
    }
    else switch( addr & 0xFFC0 ) {
        case 0x5080:
            {
                unsigned char result = dip_switches_;

                if( addr == 0x5080 ) {
                    result |= prot[ prot_index_hi ] & 0xC0;
                }

                return result;
            }
            break;
        case 0x50C0:
            {
                unsigned char result = 0;

                if( addr == 0x50C0 ) {
                    result = prot[ prot_index_hi ] & 0x3F;
                }

                return result;
            }
            break;
    }

    return PacmanBoard::readByte( addr );
}

void CrushRollerBoard::writeByte( unsigned addr, unsigned char b )
{
    addr &= 0x7FFF;

    if( addr == 0x5004 ) {
        if( b == 0 ) {
            prot_index_lo = 0;
            prot_index_hi = 0;
        }
        else {
            prot_index_lo++;
            if( prot_index_lo >= 60 ) {
                prot_index_lo = 0;
                prot_index_hi++;
                if( prot_index_hi >= 30 ) {
                    prot_index_hi = 0;
                }
            }
        }
    }
    else if( addr == 0x5007 ) {
    }
    else if( addr >= 0x4000 ) {
        PacmanBoard::writeByte( addr, b );
    }
}

bool CrushRoller::initialize( TMachineDriverInfo * info )
{
    static TResourceHandlerReplaceInfo ri[] = {
        { Ef6E, "crushkrl.6e", 0x0000000 },
        { Ef6F, "crushkrl.6f", 0x0000000 },
        { Ef6H, "crushkrl.6h", 0x0000000 },
        { Ef6J, "crushkrl.6j", 0x0000000 },
        { Ef5E, "maketrax.5e", 0x0000000 },
        { Ef5F, "maketrax.5f", 0x0000000 },
        { Ef4A, "2s140.4a", 0x00000000 }
    };

    Puckman::initialize( info );

    resourceHandler()->replace( ri, sizeof(ri) / sizeof(ri[0]) );

    resourceHandler()->addToMachineDriverInfo( info );

    // Setup user interface
    TUserInterface * ui = info->userInterface();

    ui->clear();

    TUiOptionGroup * group = ui->addGroup( S_DSW_1, dtDipSwitch );

    TUiOption * option = group->add( OptCoinage, S_Coin, 1, 0x03 );
    option->add( S_FreePlay, 0x00 );
    option->add( S_1Coin1Play, 0x01 );
    option->add( S_1Coin2Play, 0x02 );
    option->add( S_2Coin1Play, 0x03 );

    option = group->add( OptLives, S_Lives, 0, 0x0C );
    option->add( "3", 0x00 );
    option->add( "4", 0x04 );
    option->add( "5", 0x08 );
    option->add( "6", 0x0C );

    option = group->add( OptDifficulty, "First pattern", 0, 0x10 );
    option->add( S_Easy, 0x10 );
    option->add( S_Hard, 0x00 );

    option = group->add( OptTeleport, "Teleport holes", 1, 0x20 );
    option->add( S_On, 0x00 );
    option->add( S_Off, 0x20 );

    option = group->add( OptCabinet, S_Cabinet, 0, 0x10 );
    option->add( S_Upright, 0x00 );
    option->add( S_Cocktail, 0x10 );

    optionHandler()->clear();

    optionHandler()->add( &board()->dip_switches_, ui, OptCoinage, OptLives, OptDifficulty, OptTeleport );
    optionHandler()->add( &board()->port1_, ui, OptCabinet );

    // Reinitialize DIP switches
    board()->dip_switches_ = 0x31;

    return true;
}

CrushRoller::CrushRoller() : Puckman( new CrushRollerBoard )
{
    registerDriver( CrushRollerInfo );
}

bool MakeTrax::initialize( TMachineDriverInfo * info )
{
    static TResourceHandlerReplaceInfo ri[] = {
        { Ef6E, "maketrax.6e", 0x0000000 },
        { Ef6F, "maketrax.6f", 0x0000000 },
        { Ef6H, "maketrax.6h", 0x0000000 },
        { Ef6J, "maketrax.6j", 0x0000000 }
    };

    CrushRoller::initialize( info );

    resourceHandler()->replace( ri, sizeof(ri) / sizeof(ri[0]) );

    resourceHandler()->addToMachineDriverInfo( info );

    setMonitorFlip( true ); // Monitor is flipped

    return true;
}

MakeTrax::MakeTrax() : CrushRoller()
{
    registerDriver( MakeTraxInfo );
}

bool Eyes::initialize( TMachineDriverInfo * info )
{
    static TResourceHandlerReplaceInfo ri[] = {
        { Ef6E, "d7", 0x0000000 },
        { Ef6F, "e7", 0x0000000 },
        { Ef6H, "f7", 0x0000000 },
        { Ef6J, "h7", 0x0000000 },
        { Ef5E, "d5", 0x0000000 },
        { Ef5F, "e5", 0x0000000 },
        { Ef4A, "82s129.4a", 0x00000000 }
    };

    Puckman::initialize( info );

    resourceHandler()->replace( ri, sizeof(ri) / sizeof(ri[0]) );

    resourceHandler()->addToMachineDriverInfo( info );

    // Setup user interface
    TUserInterface * ui = info->userInterface();

    ui->clear();

    TUiOptionGroup * group = ui->addGroup( S_DSW_1, dtDipSwitch );

    TUiOption * option = group->add( OptCoinage, S_Coin, 1, 0x03 );
    option->add( S_FreePlay, 0x00 );
    option->add( S_1Coin1Play, 0x03 );
    option->add( S_1Coin2Play, 0x02 );
    option->add( S_2Coin1Play, 0x01 );

    option = group->add( OptLives, S_Lives, 1, 0x0C );
    option->add( "2", 0x0C );
    option->add( "3", 0x08 );
    option->add( "4", 0x04 );
    option->add( "5", 0x00 );

    option = group->add( OptBonus, "Bonus life at", 0, 0x30 );
    option->add( "50K", 0x30 );
    option->add( "75K", 0x20 );
    option->add( "100K", 0x10 );
    option->add( "125K", 0x00 );

    option = group->add( OptCabinet, S_Cabinet, 0, 0x40 );
    option->add( S_Upright, 0x40 );
    option->add( S_Cocktail, 0x00 );

    optionHandler()->clear();

    optionHandler()->add( &board()->dip_switches_, ui, OptCoinage, OptLives, OptBonus, OptCabinet );

    // Action buttons are also used
    eventHandler()->add( idKeyP1Action1, ptInverted, &board()->port2_, 0x10 );
    eventHandler()->add( idKeyP2Action1, ptInverted, &board()->port2_, 0x80 );

    return true;
}

Eyes::Eyes() : Puckman( new PacmanBoard )
{
    registerDriver( EyesInfo );
}

/*
    The code to decrypt the Eyes ROMs derives from the
    MAME machine driver.
*/
bool Eyes::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    unsigned char x_buf[0x1000];

    if( (id >= Ef6E) && (id <= Ef6J) ) {
        // ROM: D3 and D5 are swapped
        assert( len <= sizeof(x_buf) );

        for( unsigned i=0; i<len; i++ ) {
            x_buf[i] = (buf[i] & ~0x28) | ((buf[i] & 0x08) << 2) | ((buf[i] & 0x20) >> 2);
        }

        buf = x_buf;
    }
    else if( (id >= Ef5E) && (id <= Ef5F) ) {
        // Graphics ROM: data lines D4 and D6 and address lines A0 and A2 are swapped
        assert( len <= sizeof(x_buf) );

        for( unsigned i=0; i<len; i++ ) {
            unsigned j = i & 0x0F;
            unsigned o = (j & ~0x05) | ((j & 0x04) >> 2) | ((j & 0x01) << 2);

            unsigned char b = buf[ (i & 0xFFF0) | o ];

            x_buf[i] = (b & ~0x50) | ((b & 0x10) << 2) | ((b & 0x40) >> 2);
        }

        buf = x_buf;
    }

    return Puckman::setResourceFile( id, buf, len );
}

/*
    The code to decrypt the Jump Shot ROMs derives from the
    MAME machine driver (jumpshot.c).
*/
static unsigned char js_decrypt( unsigned addr, unsigned char e )
{
	static const unsigned char swap_xor_table[6][9] =
	{
		{ 7,6,5,4,3,2,1,0, 0x00 },
		{ 7,6,3,4,5,2,1,0, 0x20 },
		{ 5,0,4,3,7,1,2,6, 0xa4 },
		{ 5,0,4,3,7,1,2,6, 0x8c },
		{ 2,3,1,7,4,6,0,5, 0x6e },
		{ 2,3,4,7,1,6,0,5, 0x4e }
	};

	static const int picktable[32] =
	{
		0,2,4,4,4,2,0,2,2,0,2,4,4,2,0,2,
		5,3,5,1,5,3,5,3,1,5,1,5,5,3,5,3
	};

	// Pick method from bits 0,2,5,7,9 of the address
	unsigned int method = picktable[
		((addr & 0x001)     ) |
		((addr & 0x004) >> 1) |
		((addr & 0x020) >> 3) |
		((addr & 0x080) >> 4) |
		((addr & 0x200) >> 5)];

	// Switch method if bit 11 of the address is set
	if ((addr & 0x800) == 0x800)
		method ^= 1;

	const unsigned char * tbl = swap_xor_table[method];

    // Swap the bits according to the table
    unsigned char result = 0;

    for( int i=7; i>=0; i-- ) {
        if( e & (1 << *tbl++) ) {
            result |= (1 << i);
        }
    }

    return result ^ *tbl;
}

bool JumpShot::initialize( TMachineDriverInfo * info )
{
    static TResourceHandlerReplaceInfo ri[] = {
        { Ef4A, "prom.4a", 0x0000000 },
        { Ef7F, "prom.7f", 0x0000000 },
        { Ef6E, "6e", 0x0000000 },
        { Ef6F, "6f", 0x0000000 },
        { Ef6H, "6h", 0x0000000 },
        { Ef6J, "6j", 0x0000000 },
        { Ef5E, "5e", 0x0000000 },
        { Ef5F, "5f", 0x0000000 }
    };

    Puckman::initialize( info );

    resourceHandler()->replace( ri, sizeof(ri) / sizeof(ri[0]) );

    resourceHandler()->addToMachineDriverInfo( info );

    // Setup user interface
    TUserInterface * ui = info->userInterface();

    ui->clear();

    TUiOptionGroup * group = ui->addGroup( S_DSW_1, dtDipSwitch );

    TUiOption * option = group->add( OptBonus, "Time", 0, 0x03 );
    option->add( "2:00", 0x02 );
    option->add( "3:00", 0x03 );
    option->add( "4:00", 0x01 );

    option = group->add( OptLives, S_FreePlay, 1, 0x10 );
    option->add( S_On,  0x00 );
    option->add( S_Off, 0x10 );

    option = group->add( OptCoinage, "Two players game", 1, 0x20 );
    option->add( S_1Coin1Play, 0x20 );
    option->add( S_2Coin1Play, 0x00 );

    optionHandler()->clear();

    optionHandler()->add( &board()->dip_switches_, ui, OptCoinage, OptLives, OptBonus );

    // Joystick is 8-way here
    joystickHandler(0)->setMode( jm8Way );
    joystickHandler(1)->setMode( jm8Way );

    // Action buttons are also used in place of the start keys
    eventHandler()->add( idKeyP1Action1, ptInverted, &board()->port2_, 0x20 );
    eventHandler()->add( idKeyP2Action1, ptInverted, &board()->port2_, 0x40 );

    // Reinitialize DIP switches
    board()->dip_switches_ = 0xFF;

    return true;
}

JumpShot::JumpShot() : Puckman( new PacmanBoard )
{
    registerDriver( JumpShotInfo );
}

bool JumpShot::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    unsigned char x_buf[0x1000];

    if( (id >= Ef6E) && (id <= Ef6J) ) {
        // Decrypt ROM
        assert( len <= sizeof(x_buf) );

        for( unsigned i=0; i<len; i++ ) {
            x_buf[i] = js_decrypt( i, buf[i] );
        }

        buf = x_buf;
    }

    return Puckman::setResourceFile( id, buf, len );
}
