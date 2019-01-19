/*
    Galaga arcade machine emulator

    Copyright (c) 2011 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "galaga.h"

// Hardware info
enum {
    ScreenWidth             = 224,
    ScreenHeight            = 288,
    ScreenColors            = 256,
    VideoFrequency          = 60,
    CpuClock                = 18432000 / 6,
    SoundClock              = CpuClock / 32,
    CpuCyclesPerFrame       = CpuClock / VideoFrequency,
};

enum {
    G3p, G3m, G2m, G2l, G3f, G2c, G4l, G4d, G4f, G5n, G2n, G1c, G1d,
    OptDifficulty, OptA4, OptDemoSounds, OptFreeze, OptRackTest, OptA7, OptCabinet, OptCoinage, OptBonus, OptLives, OptShipHit
};

// Machine info
static TMachineInfo GalagaInfo = { 
    "galaga", "Galaga", S_Namco, 1981, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TGameRegistrationHandler reg( &GalagaInfo, Galaga::createInstance );

bool Galaga::initialize( TMachineDriverInfo * info )
{
    resourceHandler()->add( G3p, "gg1_1b.3p", 0x1000, efROM, main_board_->rom_+ 0x0000 );
    resourceHandler()->add( G3m, "gg1_2b.3m", 0x1000, efROM, main_board_->rom_+ 0x1000 );
    resourceHandler()->add( G2m, "gg1_3.2m",  0x1000, efROM, main_board_->rom_+ 0x2000 );
    resourceHandler()->add( G2l, "gg1_4b.2l", 0x1000, efROM, main_board_->rom_+ 0x3000 );
    
    resourceHandler()->add( G3f, "gg1_5b.3f", 0x1000, efROM, main_board_->aux_board_->rom_+ 0x0000 );

    resourceHandler()->add( G2c, "gg1_7b.2c", 0x1000, efROM, main_board_->sound_board_->rom_+ 0x0000 );
    
    resourceHandler()->add( G4l, "gg1_9.4l",  0x1000, efVideoROM, video_rom_+ 0x0000 );
    resourceHandler()->add( G4d, "gg1_11.4d", 0x1000, efVideoROM, video_rom_+ 0x1000 );
    resourceHandler()->add( G4f, "gg1_10.4f", 0x1000, efVideoROM, video_rom_+ 0x2000 );

    resourceHandler()->add( G5n, "prom-5.5n",  0x20, efPalettePROM, palette_prom_ );
    
    resourceHandler()->add( G2n, "prom-4.2n", 0x100, efPROM, palette_lookup_char_prom_ );
    resourceHandler()->add( G1c, "prom-3.1c", 0x100, efPROM, palette_lookup_sprite_prom_ );
    
    resourceHandler()->add( G1d, "prom-1.1d", 0x100, efSoundPROM, 0 );
    
    resourceHandler()->assignToMachineDriverInfo( info );
    
    // Setup user interface
    TUserInterface * ui = info->userInterface();
    
    TUiOptionGroup * group = ui->addGroup( S_DSW_A, dtDipSwitch );
    
    TUiOption * option = group->add( OptDifficulty, "Difficulty level", 0, 0x03 );
    option->add( "Rank 'A' - Easiest",        0x03 );
    option->add( "Rank 'B' - 2nd level",      0x00 );
    option->add( "Rank 'C' - 3rd level",      0x01 );
    option->add( "Rank 'D' - Most difficult", 0x02 );

    option = group->add( OptA4, "SW#3", 0, 0x04 );
    option->add( S_Off, 0x04 );
    option->add( S_On,  0x00 );

    option = group->add( OptDemoSounds, "Sound in attract mode", 1, 0x08 );
    option->add( S_Off, 0x08 );
    option->add( S_On,  0x00 );
    
    option = group->add( OptFreeze, "Freeze video", 0, 0x10 );
    option->add( S_Off, 0x10 );
    option->add( S_On,  0x00 );
    
    option = group->add( OptRackTest, "Automatic rack advance", 0, 0x20 );
    option->add( S_Off, 0x20 );
    option->add( S_On,  0x00 );
    
    option = group->add( OptA7, "SW#7", 0, 0x40 );
    option->add( S_Off, 0x40 );
    option->add( S_On,  0x00 );
    
    option = group->add( OptCabinet, S_Cabinet, 0, 0x80 );
    option->add( S_Upright,  0x80 );
    option->add( S_Cocktail, 0x00 );
    
    optionHandler()->add( &main_board_->dsw_a_, ui, OptDifficulty, OptA4, OptDemoSounds, OptFreeze, OptRackTest );
    optionHandler()->add( &main_board_->dsw_a_, ui, OptA7, OptCabinet );

    group = ui->addGroup( S_DSW_B, dtDipSwitch );
    
    option = group->add( OptCoinage, S_Coinage, 6, 0x07 );
    option->add( S_4Coin1Play, 0x04 );
    option->add( S_3Coin1Play, 0x02 );
    option->add( S_2Coin1Play, 0x06 );
    option->add( S_2Coin3Play, 0x01 );
    option->add( S_1Coin3Play, 0x05 );
    option->add( S_1Coin2Play, 0x03 );
    option->add( S_1Coin1Play, 0x07 );
    option->add( S_FreePlay,   0x00 );
    
    // Values in thousands (20 = 20,000), the second set of values applies when player starts with 5 fighters
    option = group->add( OptBonus, "Bonus ships", 1, 0x38 );
    option->add( "20,60,+60 / 30,100,+100", 0x20 );
    option->add( "20,70,+70 / 30,120,+120", 0x10 );
    option->add( "20,80,+80 / 30,150,+150", 0x30 );
    option->add( "30,100,+100 / 30,100",    0x08 );
    option->add( "30,120,+120 / 30,120",    0x28 );
    option->add( "20,60 / 30,150",          0x18 );
    option->add( "30,80 / 30",              0x28 );
    option->add( "None",                    0x00 );
    
    option = group->add( OptLives, S_Lives, 1, 0xC0 );
    option->add( "2", 0x00 );
    option->add( "3", 0x80 );
    option->add( "4", 0x40 );
    option->add( "5", 0xC0 );
    
    optionHandler()->add( &main_board_->dsw_b_, ui, OptCoinage, OptBonus, OptLives );

    group = ui->addGroup( "Cheat", dtCheat );
    
    option = group->add( OptShipHit, "Ship cannot be hit", 1, 0x01 );
    option->add( S_Enabled,  0x01 );
    option->add( S_Disabled, 0x00 );
    
    optionHandler()->add( &main_board_->cheat_, ui, OptShipHit );
    
    return true;
}

GalagaMainBoard::GalagaMainBoard() : 
    namco51xx( &port_0_, &port_1_ ),
    sound_chip_( SoundClock )
{
    cpu_ = new Z80(*this);
    
    aux_board_ = new GalagaAuxBoard(this);
    sound_board_ = new GalagaSoundBoard(this);
    
    cpu_2_ = aux_board_->cpu_;
    cpu_3_ = sound_board_->cpu_;
    
    reset();

    cheat_ = 0;

    dsw_a_ = 0xF7;
    dsw_b_ = 0x97;
    
    port_0_ = 0xFF;
    port_1_ = 0xFF;

    // The 54xx chip is not emulated yet, so explosion is simulated with a circuit derived by the Galaxian sound board
    a_noise_ = new AWhiteNoise( 48000 ); // 24000/48000 are both passable
    a_noise_->setOutput( 3.4, 0.3 );
    a_hit_latch_ = new ALatch; // Hit (explosion)
    a_hit_diode_ = new AClipperLo( *a_hit_latch_, 0.7 );
    a_hit_ = new ACapacitorWithSwitch( *a_hit_diode_, *a_noise_, Kilo(1), Kilo(100), Micro(2.2) );
    // There are three active bandpass filters on the board, only one is used here
    if( 0 ) a_hit_filter_ = new AActiveBandPassFilter( *a_hit_, Kilo(100), Kilo(22), Kilo(220), Micro(0.001), Micro(0.001) ); // R24, R23, R22, C31, C30, LM324
    if( 0 ) a_hit_filter_ = new AActiveBandPassFilter( *a_hit_, Kilo(47), Kilo(10), Kilo(150), Micro(0.01), Micro(0.01) ); // R33, R34, R35, C29, C28, LM324
    if( 1 ) a_hit_filter_ = new AActiveBandPassFilter( *a_hit_, Kilo(150), Kilo(22), Kilo(470), Micro(0.01), Micro(0.01) ); // R42, R41, R40, C27, C26, LM324
    a_hit_filter_->setGain( 1e+02 );
}

GalagaMainBoard::~GalagaMainBoard()
{
    delete a_hit_filter_;
    delete a_hit_;
    delete a_hit_diode_;
    delete a_hit_latch_;
    delete a_noise_;

    delete sound_board_;
    delete aux_board_;
    delete cpu_;
}

void GalagaMainBoard::reset()
{
    cpu_->reset();
    cpu_2_->reset();;
    cpu_3_->reset();
    
    cpu1_int_enabled_ = false;
    cpu2_int_enabled_ = false;
    cpu3_int_enabled_ = true;
    halt_cpu23_ = true;
    
    unsigned char v = 0x00;
    memset( video_ram_, v, sizeof(video_ram_) );
    memset( ram1_, v, sizeof(ram1_) );
    memset( ram2_, v, sizeof(ram2_) );
    memset( ram3_, v, sizeof(ram3_) );
    
    frame_count_ = 0;
    
    namco05xx.reset();
    namco51xx.reset();
    
    namco06xx_control_ = 0;
    namco06xx_nmi_counter_ = 0;
}

unsigned char GalagaMainBoard::readByte( unsigned addr ) 
{
    addr &= 0xFFFF;
    
    if( addr < 0x4000 ) {
        return rom_[addr];
    }
    else if( addr >= 0x6800 && addr < 0x6808 ) {
        unsigned bit_index = addr - 0x6800;
        unsigned char b1 = (dsw_a_ >> bit_index) & 1;
        unsigned char b0 = (dsw_b_ >> bit_index) & 1;

        return (b1 << 1) | b0;
    }
    else if( addr >= 0x8000 && addr < 0x8800 ) {
        return video_ram_[addr - 0x8000];
    }
    else if( addr >= 0x8800 && addr < 0x8C00 ) {
        return ram1_[addr - 0x8800];
    } 
    else if( addr >= 0x9000 && addr < 0x9400 ) {
        return ram2_[addr - 0x9000];
    } 
    else if( addr >= 0x9800 && addr < 0x9C00 ) {
        return ram3_[addr - 0x9800];
    }
    else if( addr >= 0x7000 && addr < 0x7010 ) {
        if( namco06xx_control_ & 1 ) {
            return namco51xx.read();
        }
    }
    else switch(addr) {
        case 0x7100:
            return namco06xx_control_;
        default:
            break;
    }
    
    return 0xFF;
}

void GalagaMainBoard::writeByte( unsigned addr, unsigned char b )
{
    addr &= 0xFFFF;
    
    if( addr >= 0x6800 && addr < 0x6820 ) {
        sound_chip_.setRegister( addr-0x6800, b );
    }
    else if( addr >= 0x8000 && addr < 0x8800 ) {
        video_ram_[addr - 0x8000] = b;
    }
    else if( addr >= 0x8800 && addr < 0x8C00 ) {
        ram1_[addr - 0x8800] = b;
    } 
    else if( addr >= 0x9000 && addr < 0x9400 ) {
        ram2_[addr - 0x9000] = b;
    } 
    else if( addr >= 0x9800 && addr < 0x9C00 ) {
        ram3_[addr - 0x9800] = b;
    }
    else if( addr >= 0x7000 && addr < 0x7100 ) {
        if( namco06xx_control_ & 1) {
            namco51xx.write( b );
        }
        else {
            if( b != 0xFF ) a_hit_latch_->setValue( 3.2 ); // Trigger explosion
        }
    }
    else if( addr >= 0xA000 && addr < 0xA006 ) {
        namco05xx.writeRegister( addr-0xA000, b );
    }
    else switch(addr) {
        case 0x6820:
            cpu1_int_enabled_ = b & 1;
            break;
        case 0x6821:
            cpu2_int_enabled_ = b & 1;
            break;
        case 0x6822:
            cpu3_int_enabled_ = ! (b & 1);
            break;
        case 0x6823:
            halt_cpu23_ = (b & 1) == 0; // Reset line connected to motion and sound CPU's
            
            if( halt_cpu23_ ) {
                cpu_2_->reset();
                cpu_3_->reset();
            }
            break;
        case 0x6830:
            // Watchdog reset
            break;
        case 0x7100:
            if( ((namco06xx_control_ & 0x0F) == 0) && ((b & 0x0F) != 0) ) {
                namco06xx_nmi_counter_ = 0;
            }
            namco06xx_control_ = b;
            break;
        case 0xA007:
            // Flip screen
            break;
        default:
            break;
    }
}

GalagaSoundBoard::GalagaSoundBoard(GalagaMainBoard * mb)
{
    main_board_ = mb;
    cpu_ = new Z80(*this);
}

GalagaSoundBoard::~GalagaSoundBoard()
{
    delete cpu_;
}

unsigned char GalagaSoundBoard::readByte( unsigned addr ) 
{
    addr &= 0xFFFF;
    
    if( addr < 0x2000 ) {
        return rom_[addr];
    }
    else if( addr >= 0x4000 ) {
        return main_board_->readByte(addr);
    }
    
    return 0xFF;
}

void GalagaSoundBoard::writeByte( unsigned addr, unsigned char b )
{
    main_board_->writeByte( addr, b );
}

GalagaAuxBoard::GalagaAuxBoard(GalagaMainBoard * mb)
{
    main_board_ = mb;
    cpu_ = new Z80(*this);
}

GalagaAuxBoard::~GalagaAuxBoard()
{
    delete cpu_;
}
   
unsigned char GalagaAuxBoard::readByte( unsigned addr ) 
{
    addr &= 0xFFFF;
    
    if( addr < 0x2000 ) {
        // Cheat: wait until ROM has been tested, then patch the ROM to disable hit detection
        if( addr == 0x0045 && rom_[addr] == 0xEE && main_board_->cheat_ && main_board_->frame_count_ >= 60*14 ) {
            return 0xBE;
        }
    
        return rom_[addr];
    }
    else if( addr >= 0x4000 ) {
        return main_board_->readByte(addr);
    }
    
    return 0xFF;
}   
   
void GalagaAuxBoard::writeByte( unsigned addr, unsigned char b )
{
    main_board_->writeByte( addr, b );
}

Galaga::Galaga() :
    char_data_( 8, 8*256 ),     // 256 8x8 characters
    sprite_data_( 16, 16*128 )  // 128 16x16 sprites
{
    main_board_ = new GalagaMainBoard();

    refresh_roms_ = true;
    
    setJoystickHandler( 0, new TJoystickToPortHandler(idJoyP1Joystick1, ptInverted, jm4Way, &main_board_->port_1_, 0x00000208) ); // Down, up, right, left
    setJoystickHandler( 1, new TJoystickToPortHandler(idJoyP2Joystick1, ptInverted, jm4Way, &main_board_->port_1_, 0x00002080) );
    // 0x01, 0x04, 0x10 and 0x40 are not used
    
    eventHandler()->add( idKeyP1Action1,     ptInverted, &main_board_->port_0_, 0x01 );
    eventHandler()->add( idKeyP2Action1,     ptInverted, &main_board_->port_0_, 0x02 );
    eventHandler()->add( idKeyStartPlayer1,  ptInverted, &main_board_->port_0_, 0x04 );
    eventHandler()->add( idKeyStartPlayer2,  ptInverted, &main_board_->port_0_, 0x08 );
    eventHandler()->add( idCoinSlot1,        ptInverted, &main_board_->port_0_, 0x10 );
    eventHandler()->add( idCoinSlot2,        ptInverted, &main_board_->port_0_, 0x20 );
    // 0x40 and 0x80 used for service
    
    createScreen( ScreenWidth, ScreenHeight, ScreenColors );
    
    registerDriver( GalagaInfo );
}

Galaga::~Galaga()
{
    delete main_board_;
}

bool Galaga::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    if( id == G1d ) {
        main_board_->sound_chip_.setSoundPROM( buf );
        return true;
    }
    
    return 0 == resourceHandler()->handle( id, buf, len );
}

void GalagaMainBoard::run()
{
    const int StepsPerFrame = 100;
    const int CpuCyclesPerStep = CpuCyclesPerFrame / StepsPerFrame;
    const int Cpu3_NMI1 = 8; // Too close to VBLANK and some sounds won't work, too far and it will freeze
    const int Cpu3_NMI2 = Cpu3_NMI1 + (StepsPerFrame / 2);
    const int VBLANK = 1;
    const int Namco06xxCyclesBeforeNMI = 600;

    unsigned cycles_1 = 0;
    unsigned cycles_2 = 0;
    unsigned cycles_3 = 0;
    
    frame_count_++;
    
    for( int step=StepsPerFrame; step>0; step-- ) {
        cycles_1 = cpu_->run( CpuCyclesPerStep - cycles_1 );
        
        if( ! halt_cpu23_ ) {
            if( cpu2_int_enabled_ && step == VBLANK ) {
                cpu_2_->interrupt(0xFF);
            }
            
            cycles_2 = cpu_2_->run( CpuCyclesPerStep - cycles_2 );
            
            if( cpu3_int_enabled_ ) {
                if( step == Cpu3_NMI1 || step == Cpu3_NMI2 ) {
                    cpu_3_->nmi();
                }
            }
            
            cycles_3 = cpu_3_->run( CpuCyclesPerStep - cycles_3 );
            
        }
        
        if( cpu1_int_enabled_ && step == VBLANK ) {
            cpu_->interrupt(0xFF);
        }
        
        if( namco06xx_control_ & 0x0F ) {
            namco06xx_nmi_counter_ += CpuCyclesPerStep;
            if( namco06xx_nmi_counter_ >= Namco06xxCyclesBeforeNMI ) {
                namco06xx_nmi_counter_ -= Namco06xxCyclesBeforeNMI;
                cpu_->nmi();
            }
        }
    }
}

void Galaga::run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate )
{
    if( refresh_roms_ ) {
        onVideoROMsChanged();
        refresh_roms_ = false;
    }
    
    main_board_->run();

    // Sound
    TMixerBuffer * mixer_buffer = frame->getMixer()->getBuffer( chMono, samplesPerFrame, 3 );
    
    main_board_->sound_chip_.setSamplingRate( samplingRate );
    main_board_->sound_chip_.playSound( mixer_buffer->data(), samplesPerFrame );
    
    // Explosion
    mixer_buffer = frame->getMixer()->getBuffer( chMono, samplesPerFrame, 1 );
    
    main_board_->a_hit_filter_->resetStream();
    main_board_->a_hit_filter_->updateTo( samplesPerFrame );
    main_board_->a_hit_filter_->mixStream( mixer_buffer->data(), 51, 0, 255 ); // Noise must be clipped
    main_board_->a_hit_latch_->setValue( 0.3 );
    
    // Render the video
    frame->setVideo( renderVideo() );
}

void Galaga::reset()
{
    main_board_->reset();
}

static const TDecodeCharInfo8x8 charLayout =
{
    2,
    { 0, 4 },
	{ 8*7, 8*6, 8*5, 8*4, 8*3, 8*2, 8*1, 8*0 },
    { 64+3, 64+2, 64+1, 64+0, 3, 2, 1, 0 }
};

static const TDecodeCharInfo spriteLayout =
{
    16, 16,
	2,
	{ 0, 4 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
    { 
      24*8+0, 24*8+1, 24*8+2, 24*8+3,
      16*8+0, 16*8+1, 16*8+2, 16*8+3,
      8*8, 8*8+1, 8*8+2, 8*8+3, 
      0, 1, 2, 3
    }
};

// Update the internal tables after a ROM has changed
void Galaga::onVideoROMsChanged()
{
    int i;
    
    // Refresh palette
    for( i=0; i<32; i++ ) {
        palette()->setColor( i, TPalette::decodeByte(palette_prom_[i]) );
    }
    
    // Character palette by lookup table (PROM values are from 0x00 to 0x0F)
    for( int i=0; i<256; i++ ) {
        palette_char_[i] = (palette_lookup_char_prom_[i] & 0x0f) | 0x10;
    }
    
    // Sprite palette by lookup table (PROM values are from 0x00 to 0x0F)
    for( int i=0; i<256; i++ ) {
        palette_sprite_[i] = (palette_lookup_sprite_prom_[i] & 0x0F);
    }
    
    // Characters (256 8x8x2 characters)
    decodeCharSet8x8( (unsigned char *) char_data_.data(), charLayout, video_rom_, 256, 16 );
    
    // Sprites (128 16x16x2 sprites)
    decodeCharSet( (unsigned char *) sprite_data_.data(), spriteLayout, video_rom_ + 0x1000, 128, 16*16*2 );
    
    // Build palette colors for the starfield
	for (i = 0;i < 64;i++) {
		static const unsigned char W[4] = { 0x00, 0x47, 0x97 ,0xDE };
        
		unsigned char r = W[(i >> 0) & 0x03];
		unsigned char g = W[(i >> 2) & 0x03];
		unsigned char b = W[(i >> 4) & 0x03];
        
		palette()->setColor(32 + i, TPalette::encodeColor(r, g, b));
	}
}

TBitmapIndexed * Galaga::renderVideo()
{
    // Clear screen
    screen()->bits()->fill( palette_lookup_char_prom_[0] );
    
    // Draw the starfield...
    main_board_->namco05xx.update();
    main_board_->namco05xx.render( screen() );
    
    // ...then add the sprites...
    unsigned char * spriteram1 = main_board_->ram1_ + 0x380;
    unsigned char * spriteram2 = main_board_->ram2_ + 0x380;
    unsigned char * spriteram3 = main_board_->ram3_ + 0x380;
    int spriteram_size = 0x80;
    
    for (int offs = 0;offs < spriteram_size;offs += 2) {
        int code = spriteram1[ offs ];
        int color = spriteram1[ offs + 1 ] & 31;
        int flipx = spriteram3[ offs ] & 2;
        int flipy = spriteram3[ offs ] & 1;
        int sy = spriteram2[ offs + 1 ] - 40 + 0x100*(spriteram3[ offs + 1 ] & 3);
        int sx = spriteram2[ offs ] - 16 - 1;
        
        TBltAddXlatSrcTrans         sprite_blitter_f(0,0,palette_lookup_sprite_prom_);
        TBltAddXlatSrcTransReverse  sprite_blitter_r(0,0,palette_lookup_sprite_prom_);
        TBltAdd * blitter;
        
        unsigned mode = opAdd | (flipy ? opFlipY : 0);
        
        if( flipx ) {
            mode |= opFlipX;
            blitter = &sprite_blitter_r;
        }
        else {
            blitter = &sprite_blitter_f;
        }
        
        color *= 4;
        
        unsigned char scale = spriteram3[ offs ] & 0x0C;
        
        switch( scale ) {
        case 0x00: // Normal
            screen()->bits()->copy( sx, sy, sprite_data_, 0, 16*code, 16, 16, mode, blitter->color(color) );
            break;
        case 0x04: // Double height
            screen()->bits()->copy( sx, sy+16, sprite_data_, 0, 16*code, 16, 16, mode, blitter->color(color) );
            screen()->bits()->copy( sx, sy, sprite_data_, 0, 16*(code+1), 16, 16, mode, blitter->color(color) );
            break;
        case 0x08: // Double width
            screen()->bits()->copy( sx+16, sy, sprite_data_, 0, 16*code, 16, 16, mode, blitter->color(color) );
            screen()->bits()->copy( sx, sy, sprite_data_, 0, 16*(code+2), 16, 16, mode, blitter->color(color) );
            break;
        case 0x0C: // Double size
            screen()->bits()->copy( sx+16, sy, sprite_data_, 0, 16*(code+0), 16, 16, mode, blitter->color(color) );
            screen()->bits()->copy( sx+16, sy+16, sprite_data_, 0, 16*(code+1), 16, 16, mode, blitter->color(color) );
            screen()->bits()->copy( sx, sy, sprite_data_, 0, 16*(code+2), 16, 16, mode, blitter->color(color) );
            screen()->bits()->copy( sx, sy+16, sprite_data_, 0, 16*(code+3), 16, 16, mode, blitter->color(color) );
            break;
        }
    }
    
    // ...and finally the character tiles
    unsigned char * video = main_board_->video_ram_;
    unsigned char * color = main_board_->video_ram_+0x400;
    
    TBltAddXlatSrcTrans char_blitter_f(0,0,palette_lookup_char_prom_);
    
    for( int y=0; y<36; y++ ) {
        for( int x=0; x<28; x++ ) {
            // Get the offset of this location in the video RAM (this is same as Pacman)
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
            
            screen()->bits()->copy( x*8, y*8, char_data_, 0, 8*video[offset], 8, 8, opAdd, char_blitter_f.color( 4*(0x3F & color[offset]) ) );
        }
    }
    
    return screen();
}
