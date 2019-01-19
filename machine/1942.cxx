/*
    1942 arcade machine emulator

    Copyright (c) 2011 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "1942.h"

// Hardware info
enum {
    ScreenWidth             = 224,
    ScreenHeight            = 256,
    ScreenColors            = 256,
    VideoFrequency          = 60,
    CpuClock                = 12000000 / 3,
    SoundCpuClock           = 12000000 / 4,
    CpuCyclesPerFrame       = CpuClock / VideoFrequency,
    CpuCyclesAfterInterrupt = 5500, // Between VBLANK and sound interrupt
    SoundCpuCyclesPerFrame  = SoundCpuClock / VideoFrequency
};

enum {
    SrM3, SrM4, SrM5, SrM6, SrM7, SrC11, SrF2, SrA1, SrA2, SrA3, SrA4, SrA5, SrA6, SrL1, SrL2, SrN1, SrN2, SrE8, SrE9, SrE10, SrF1, SrD6, SrK3,
    OptLives, OptBonus, OptCoinage, OptDifficulty, OptTest
};

// Machine info
static TMachineInfo M1942Info = { 
    "1942", "1942", S_Taito, 1984, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};
 
static TGameRegistrationHandler reg( &M1942Info, M1942::createInstance );

bool M1942::initialize( TMachineDriverInfo * info )
{
    resourceHandler()->add( SrM3, "srb-03.m3", 0x4000, efROM, main_board_->rom_+ 0x0000 );
    resourceHandler()->add( SrM4, "srb-04.m4", 0x4000, efROM, main_board_->rom_+ 0x4000 );
    resourceHandler()->add( SrM5, "srb-05.m5", 0x4000, efROM, main_board_->rom_+ 0x8000 ); // Banked
    resourceHandler()->add( SrM6, "srb-06.m6", 0x2000, efROM, main_board_->rom_+ 0xC000 ); // Banked
    resourceHandler()->add( SrM7, "srb-07.m7", 0x4000, efROM, main_board_->rom_+0x10000 ); // Banked

    resourceHandler()->add( SrC11, "sr-01.c11", 0x4000, efROM, sound_board_.rom() );

    resourceHandler()->add( SrF2, "sr-02.f2", 0x2000, efVideoROM, char_rom_ );

    resourceHandler()->add( SrA1, "sr-08.a1", 0x2000, efVideoROM, tile_rom_ );
    resourceHandler()->add( SrA2, "sr-09.a2", 0x2000, efVideoROM, tile_rom_+0x2000 );
    resourceHandler()->add( SrA3, "sr-10.a3", 0x2000, efVideoROM, tile_rom_+0x4000 );
    resourceHandler()->add( SrA4, "sr-11.a4", 0x2000, efVideoROM, tile_rom_+0x6000 );
    resourceHandler()->add( SrA5, "sr-12.a5", 0x2000, efVideoROM, tile_rom_+0x8000 );
    resourceHandler()->add( SrA6, "sr-13.a6", 0x2000, efVideoROM, tile_rom_+0xA000 );

    resourceHandler()->add( SrL1, "sr-14.l1", 0x4000, efVideoROM, sprite_rom_ );
    resourceHandler()->add( SrL2, "sr-15.l2", 0x4000, efVideoROM, sprite_rom_+0x4000 );
    resourceHandler()->add( SrN1, "sr-16.n1", 0x4000, efVideoROM, sprite_rom_+0x8000 );
    resourceHandler()->add( SrN2, "sr-17.n2", 0x4000, efVideoROM, sprite_rom_+0xC000 );

    resourceHandler()->add( SrE8,  "sb-5.e8",  0x100, efPalettePROM, palette_prom_ );       // Red
    resourceHandler()->add( SrE9,  "sb-6.e9",  0x100, efPalettePROM, palette_prom_+0x100 ); // Green
    resourceHandler()->add( SrE10, "sb-7.e10", 0x100, efPalettePROM, palette_prom_+0x200 ); // Blue

    resourceHandler()->add( SrF1, "sb-0.f1",  0x100, efPROM, palette_lookup_char_prom_ );
    resourceHandler()->add( SrD6, "sb-4.d6",  0x100, efPROM, palette_lookup_tile_prom_ );
    resourceHandler()->add( SrK3, "sb-8.k3",  0x100, efPROM, palette_lookup_sprite_prom_ );

    resourceHandler()->assignToMachineDriverInfo( info );
    
    // Setup user interface
    TUserInterface * ui = info->userInterface();
    
    TUiOptionGroup * group = ui->addGroup( S_DSW_A, dtDipSwitch );
    
    TUiOption * option = group->add( OptLives, S_Lives, 2, 0xC0 );
    option->add( "1", 0x80 );
    option->add( "2", 0x40 );
    option->add( "3", 0xC0 ); // Default
    option->add( "5", 0x00 );
    
    option = group->add( OptBonus, "Bonus points", 3, 0x30 );
    option->add( "30,000/100,000+", 0x00 );
    option->add( "30,000/80,000+",  0x10 );
    option->add( "20,000/100,000+", 0x20 );
    option->add( "20,000/80,000+",  0x30 ); // Default
    
    option = group->add( OptCoinage, S_Coin, 7, 0x07 );
    option->add( S_FreePlay,    0x00 );
    option->add( S_4Coin1Play,  0x01 );
    option->add( S_3Coin1Play,  0x02 );
    option->add( S_2Coin3Play,  0x03 );
    option->add( S_2Coin1Play,  0x04 );
    option->add( S_1Coin4Play,  0x05 );
    option->add( S_1Coin2Play,  0x06 );
    option->add( S_1Coin1Play,  0x07 ); // Default
    
    optionHandler()->add( &main_board_->dsw_a_, ui, OptLives, OptBonus, OptCoinage );
    
    group = ui->addGroup( S_DSW_B, dtDipSwitch );
    
    option = group->add( OptDifficulty, "Difficulty", 3, 0x60 );
    option->add( "Very difficult",  0x00 );
    option->add( S_Difficult,       0x20 );
    option->add( S_Easy,            0x40 );
    option->add( S_Normal,          0x60 ); // Default
    
    option = group->add( OptTest, "Test mode", 0, 0x08 );
    option->add( S_Off, 0x08 );
    option->add( S_On,  0x00 );
    
    optionHandler()->add( &main_board_->dsw_b_, ui, OptDifficulty, OptTest );
    
    return true;
}

bool M1942::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    return 0 == resourceHandler()->handle( id, buf, len );
}

M1942::M1942( M1942MainBoard * board ) :
    char_data_( 8, 8*512 ),     // 512 8x8 characters
    tile_data_( 16, 16*512 ),   // 512 16x16 tiles
    sprite_data_( 16, 16*512 )  // 512 16x16 sprites
{
    main_board_ = board;

    refresh_roms_ = true;

    setJoystickHandler( 0, new TJoystickToPortHandler(idJoyP1Joystick1, ptInverted, jm4Way, &board->port1_, 0x04080102) ); // Down, up, right, left
    setJoystickHandler( 1, new TJoystickToPortHandler(idJoyP2Joystick1, ptInverted, jm4Way, &board->port2_, 0x04080102) );

    eventHandler()->add( idKeyP1Action1,     ptInverted, &main_board_->port1_, 0x10 );
    eventHandler()->add( idKeyP1Action2,     ptInverted, &main_board_->port1_, 0x20 );
    eventHandler()->add( idKeyP2Action1,     ptInverted, &main_board_->port2_, 0x10 );
    eventHandler()->add( idKeyP2Action2,     ptInverted, &main_board_->port2_, 0x20 );
    
    eventHandler()->add( idCoinSlot1,        ptInverted, &board->port0_, 0x80 );
    eventHandler()->add( idCoinSlot2,        ptInverted, &board->port0_, 0x40 );
    eventHandler()->add( idKeyStartPlayer1,  ptInverted, &board->port0_, 0x01 );
    eventHandler()->add( idKeyStartPlayer2,  ptInverted, &board->port0_, 0x02 );
    
    createScreen( ScreenWidth, ScreenHeight, ScreenColors );

    registerDriver( M1942Info );
}

M1942SoundBoard::M1942SoundBoard() : Z80_AY3_SoundBoard( 2, SoundCpuClock / 2, 0x4000, 0x0800, 0x4000 )
{
}

void M1942SoundBoard::run()
{
    Z80_AY3_SoundBoard::run();
}

unsigned char M1942SoundBoard::onReadByte( unsigned addr )
{
    if( addr == 0x6000 ) {
        return sound_command_;
    }
    
    return 0;
}

void M1942SoundBoard::onWriteByte( unsigned addr, unsigned char value )
{
    switch( addr ) {
        case 0x8000:
            soundChip(0)->writeAddress(value);
            break;
        case 0x8001:
            soundChip(0)->writeData(value);
            break;
        case 0xC000:
            soundChip(1)->writeAddress(value);
            break;
        case 0xC001:
            soundChip(1)->writeData(value);
            break;
    }
}

void M1942SoundBoard::onAfterSamplingStep( unsigned step )
{
    // There are 12 sampling steps by default and we need 4 interrupts per frame
    if( (step % 3) == 2 ) {
        cpu()->interrupt( 0x00 );
    }
}

unsigned char M1942SoundBoard::readPort( unsigned addr )
{
    return 0;
}

void M1942SoundBoard::writePort( unsigned addr, unsigned char value )
{
}

M1942MainBoard::M1942MainBoard()
{
    // Initialize the CPU and the RAM
    cpu_ = new Z80( *this );

    memset( rom_+0xE000, 0xFF, 0x2000 );
    memset( ram_, 0, sizeof(ram_) );
    memset( video_ram_, 0, sizeof(video_ram_) );
    memset( sprite_ram_, 0, sizeof(sprite_ram_) );
    
    // Initialize the board variables
    curr_rom_bank_ = rom_;
    port0_ = 0xFF;
    port1_ = 0xFF;
    port2_ = 0xFF;
    dsw_a_ = 0xFF;
    dsw_b_ = 0xFF;

    sound_command_ = 0;
    sound_reset_ = false;
    palette_bank_ = 0;
    scroll_ = 0;

    // Reset the machine
    reset();
}

void M1942MainBoard::reset()
{
    cpu_->reset();
}

void M1942MainBoard::run()
{
    sound_reset_ = false;
    
    // Run the main CPU
    cpu_->run( CpuCyclesPerFrame - CpuCyclesAfterInterrupt );
    cpu_->interrupt( 0xD7 ); // RST 10h
    cpu_->run( CpuCyclesAfterInterrupt );
    cpu_->interrupt( 0xCF ); // RST 08h
}

unsigned char M1942MainBoard::readByte( unsigned addr ) 
{
    addr &= 0xFFFF;
    
    if( addr < 0x8000 ) {
        return rom_[addr];
    }
    else if( addr < 0xC000 ) {
        return curr_rom_bank_[ addr ]; // 8000-BFFF contains a ROM bank
    }
    else if( addr >= 0xCC00 && addr < 0xCC7F ) {
        return sprite_ram_[ addr-0xCC00 ];
    }
    else if( addr >= 0xD000 && addr < 0xDC00 ) {
        return video_ram_[ addr-0xD000 ];
    }
    else if( addr >= 0xE000 && addr < 0xF000 ) {
        return ram_[ addr-0xE000 ];
    }
    else switch( addr ) {
        case 0xC000:
            return port0_;
        case 0xC001:
            return port1_;
        case 0xC002:
            return port2_;
        case 0xC003:
            return dsw_a_;
        case 0xC004:
            return dsw_b_;
    }

    return 0xFF;
}

void M1942MainBoard::writeByte( unsigned addr, unsigned char b )
{
    addr &= 0xFFFF;

    if( addr >= 0xCC00 && addr < 0xCC7F ) {
        sprite_ram_[ addr-0xCC00 ] = b;
    }
    else if( addr >= 0xD000 && addr < 0xDC00 ) {
        video_ram_[ addr-0xD000 ] = b;
    }
    else if( addr >= 0xE000 && addr < 0xF000 ) {
        ram_[ addr-0xE000 ] = b;
    }
    else switch( addr ) {
        case 0xC800:
            sound_command_ = b;
            break;
        case 0xC802: 
            scroll_ = (scroll_ & 0xFF00) | b;
            break;
        case 0xC803:
            scroll_ = (scroll_ & 0x00FF) | (((unsigned)b) << 8);
            break;
        case 0xC804:
            // Bit 0 is for the coin counter
            if( b & 0x10 ) sound_reset_ = true;
            break;
        case 0xC805:
            palette_bank_ = b & 3;
            break;
        case 0xC806:
            curr_rom_bank_ = rom_ + (b & 3) * 0x4000;
            break;
    }
}

void M1942::reset()
{
    main_board_->reset();

    sound_board_.reset();
}

void M1942::run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate )
{
    if( refresh_roms_ ) {
        onVideoROMsChanged();
        refresh_roms_ = false;
    }

    // Run the main CPU
    main_board_->run();
    
    // Run the sound CPU
    if( main_board_->sound_reset_ ) sound_board_.reset();
    sound_board_.sound_command_ = main_board_->sound_command_;
    sound_board_.run();
    sound_board_.playSound( SoundCpuCyclesPerFrame, frame->getMixer(), samplesPerFrame, samplingRate );
    
    // Render the video
    frame->setVideo( renderVideo() );
}

TBitmapIndexed * M1942::renderVideo()
{
    unsigned char * video_ram = main_board_->video_ram_;
    
    // Draw the background...
    TBltAddXlat         bg_blitter_f(0,palette_tile_[main_board_->palette_bank_]);
    TBltAddXlatReverse  bg_blitter_r(0,palette_tile_[main_board_->palette_bank_]);

    for( int cy=0; cy<32; cy++ ) {
        // Maybe I'm not used to this kind of hardware but it's taken an awful lot of time to get the darn thing right!
        unsigned offset = 0x800 + 32*(31-cy);
        for( int cx=0; cx<16; cx++ ) {
            // Get the offset of this location in the video RAM
            unsigned v = video_ram[offset];
            unsigned c = video_ram[offset+0x10];
            unsigned m = 0;
            offset++;

            if( c & 0x40 ) m |= opFlipX;
            if( c & 0x20 ) m |= opFlipY;

            TBltAdd * bg_blitter = (m & opFlipX) ? &bg_blitter_r : &bg_blitter_f;
            
            v |= (c & 0x80) << 1; // 9 bits for a tile
            c &= 0x1F;            // 32 colors per tile

            int x = cx*16;
            int y = cy*16;
            
            x -= 16;    // Center screen
            y -= 256;   // Only half of the screen is visible
            y += main_board_->scroll_;
            
            // Wrap around if needed
            while( y >= 512 ) y -= 512; 
            if( y > (512-16) ) { // Wrap this tile around
                screen()->bits()->copy( x, y-512, tile_data_, 0, 16*v, 16, 16, m, bg_blitter->color(c*8) );
            }

            screen()->bits()->copy( x, y, tile_data_, 0, 16*v, 16, 16, m, bg_blitter->color(c*8) );
        }
    }

    // ...then draw the foreground characters...
    TBltAddXlatSrcTrans fg_blitter_f(0,0,palette_char_);
    
    for( int cx=0; cx<32; cx++ ) {
        for( int cy=0; cy<32; cy++ ) {
            // Get the offset of this location in the video RAM
            unsigned offset = 32*cx + (31-cy);
            
            unsigned v = video_ram[offset];
            unsigned c = video_ram[offset+0x400];
            
            v |= (c & 0x80) << 1; // Characters use 9 bits
            c &= 0x3F;            // Max 64 colors
            
            int x = cx*8;
            int y = cy*8;
            
            x -= 16;

            screen()->bits()->copy( x, y, char_data_, 0, 8*v, 8, 8, opAdd, fg_blitter_f.color(c*4) );
        }
    }
    
    // ...and finally the sprites!
    video_ram = main_board_->sprite_ram_;
    
    TBltAddXlatSrcTrans sp_blitter_f(0,0xF,palette_sprite_);
    
    for( int i=0; i<32; i++ ) {
        unsigned v = *video_ram++;
        unsigned c = *video_ram++;
        int x = *video_ram++;
        int y = *video_ram++;
        int h = (c & 0xC0) >> 6; // Sprite height (0=single, 1=double, 2=quad)
        
        // Adjust data
        y -= 16*(c & 0x10);
        v = (v & 0x7F) | ((v & 0x80) << 1) | ((c & 0x20) << 2);
        c &= 0x0F;
        
        y = 256-y;
        x -= 16;

        h = 1 << h;
        while( h > 0 ) {
            screen()->bits()->copy( x, y-16, sprite_data_, 0, 16*v, 16, 16, opAdd, sp_blitter_f.color(c*16) );
            x += 16;
            v++;
            h--;
        }
    }
    
    return screen();
}

static unsigned decodePaletteComponent( unsigned char b )
{
    unsigned char bit0 = (b     ) & 0x01;
    unsigned char bit1 = (b >> 1) & 0x01;
    unsigned char bit2 = (b >> 2) & 0x01;
    unsigned char bit3 = (b >> 3) & 0x01;
    
    return 0x0E*bit0 + 0x1F*bit1 + 0x43*bit2 + 0x8F*bit3;
}

static const TDecodeCharInfo8x8 charLayout =
{
    2,
    { 4, 0 },
    { 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	{ 8+0, 8+1, 8+2, 8+3, 0, 1, 2, 3 },
};

static const TDecodeCharInfo tileLayout =
{
    16, 16,
	3,
	{ 2*8*0x4000, 1*8*0x4000, 0*8*0x4000 },
	{ 15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8, 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	{ 7, 6, 5, 4, 3, 2, 1, 0, 16*8+7, 16*8+6, 16*8+5, 16*8+4, 16*8+3, 16*8+2, 16*8+1, 16*8+0 }
};

static const TDecodeCharInfo spriteLayout =
{
    16, 16,
	4,
	{ 4, 0, 4+8*0x8000, 0+8*0x8000 },
	{ 15*16, 14*16, 13*16, 12*16, 11*16, 10*16, 9*16, 8*16, 7*16, 6*16, 5*16, 4*16, 3*16, 2*16, 1*16, 0*16 },
    { 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0, 16*16+3, 16*16+2, 16*16+1, 16*16+0, 16*16+8+3, 16*16+8+2, 16*16+8+1, 16*16+8+0 }
};

// Update the internal tables after a ROM has changed
void M1942::onVideoROMsChanged()
{
    int i;
    
    // Refresh palette
    for( i=0; i<256; i++ ) {
        unsigned r = decodePaletteComponent( palette_prom_[i      ] );
        unsigned g = decodePaletteComponent( palette_prom_[i+0x100] );
        unsigned b = decodePaletteComponent( palette_prom_[i+0x200] );
            
        palette()->setColor( i, TPalette::encodeColor(r,g,b) );
    }
    
    // Character palette by lookup table (PROM values are from 0x00 to 0x0F)
    for( int i=0; i<256; i++ ) {
        palette_char_[i] = palette_lookup_char_prom_[i] | 0x80;
    }
    
    // Tile palette by lookup table (PROM values are from 0x00 to 0x0F)
    for( int i=0; i<256; i++ ) {
        for( int j=0; j<4; j++ ) {
            palette_tile_[j][i] = palette_lookup_tile_prom_[i] | (j*0x10);
        }
    }
    
    // Sprite palette by lookup table (PROM values are from 0x00 to 0x0F)
    for( int i=0; i<256; i++ ) {
        palette_sprite_[i] = palette_lookup_sprite_prom_[i] | 0x40;
    }
    
    // Characters (512 8x8x2 characters)
    decodeCharSet8x8( (unsigned char *) char_data_.data(), charLayout, char_rom_, 512, 16 );
    
    // Tiles (512 16x16x3 tiles)
    decodeCharSet( (unsigned char *) tile_data_.data(), tileLayout, tile_rom_, 512, 16*16 );
    
    // Sprites (512 16x16x4 sprites)
    decodeCharSet( (unsigned char *) sprite_data_.data(), spriteLayout, sprite_rom_, 512, 16*16*2 );
}
