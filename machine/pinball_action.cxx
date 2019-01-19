/*
    Pinball Action arcade machine emulator

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "pinball_action.h"

// Output flip-flops
enum {
    FlipScreen          = 0x01,
    InterruptEnabled    = 0x08,
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
    CpuClock                = 4000000,
    SoundCpuClock           = 3072000,
    CpuCyclesPerFrame       = CpuClock / VideoFrequency,
    SoundCpuCyclesPerFrame  = SoundCpuClock / VideoFrequency
};

enum { 
    EfP7, EfN7, EfL7, EfE3, EfS6, EfS7, EfS8, EfJ5, EfJ6, EfJ7, EfJ8, EfC7, EfD7, EfF7,
    OptCoin1, OptCoin2, OptBalls, OptDemoSounds, OptCabinet, OptExtendPlay, OptExtraBall, OptFlippers, OptOutlanes
};

// Machine info
static TMachineInfo PinballActionInfo = { 
    "pbaction", "Pinball Action", S_Tehkan, 1985, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TGameRegistrationHandler reg( &PinballActionInfo, PinballAction::createInstance );

bool PinballAction::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    // Need to refresh some internal tables if the video/color ROMs change
    refresh_roms_ |= (id >= EfS6);

    return 0 == resourceHandler()->handle( id, buf, len );
}

PinballAction::PinballAction() :
    main_board_( &sound_board_ ),
    bg_char_data_( 8, 8*2048 ), // 2048 8x8 characters 4bpp
    fg_char_data_( 8, 8*1024 ), // 1024 8x8 characters 3bpp
    sm_sprite_data_( 16, 16*128 ), // 128 16x16 sprites 3bpp
    lg_sprite_data_( 32, 32*32 ) // 32 32x32 sprites 3bpp
{
    createScreen( ScreenWidth, ScreenHeight, ScreenColors );

    refresh_roms_ = false;

    eventHandler()->add( idCoinSlot1,       ptNormal, &main_board_.port2_, 0x02 );
    eventHandler()->add( idCoinSlot2,       ptNormal, &main_board_.port2_, 0x01 );
    eventHandler()->add( idKeyStartPlayer1, ptNormal, &main_board_.port2_, 0x04 );
    eventHandler()->add( idKeyStartPlayer2, ptNormal, &main_board_.port2_, 0x08 );
    
    eventHandler()->add( idKeyP1Action3,    ptNormal, &main_board_.port0_, 0x01 );
    eventHandler()->add( idKeyP1Action2,    ptNormal, &main_board_.port0_, 0x04 );
    eventHandler()->add( idKeyP1Action1,    ptNormal, &main_board_.port0_, 0x08 );
    eventHandler()->add( idKeyP1Action4,    ptNormal, &main_board_.port0_, 0x10 );

    setJoystickHandler( 0, new TJoystickToPortHandler( idJoyP1Joystick1, ptNormal, jm4Way, &main_board_.port1_, 0x08040000 ) );

    registerDriver( PinballActionInfo );
}

bool PinballAction::initialize( TMachineDriverInfo * info )
{
    // Declare resources
    resourceHandler()->add( EfP7, "b-p7.bin", 0x4000, efROM,      main_board_.rom_+0x0000 );
    resourceHandler()->add( EfN7, "b-n7.bin", 0x4000, efROM,      main_board_.rom_+0x4000 );
    resourceHandler()->add( EfL7, "b-l7.bin", 0x2000, efROM,      main_board_.rom_+0x8000 );
    resourceHandler()->add( EfE3, "a-e3.bin", 0x2000, efROM,      sound_board_.rom_+0x0000 );
    
    resourceHandler()->add( EfS6, "a-s6.bin", 0x2000, efVideoROM, video_rom_s_+0x0000 );
    resourceHandler()->add( EfS7, "a-s7.bin", 0x2000, efVideoROM, video_rom_s_+0x2000 );
    resourceHandler()->add( EfS8, "a-s8.bin", 0x2000, efVideoROM, video_rom_s_+0x4000 );
    resourceHandler()->add( EfJ5, "a-j5.bin", 0x4000, efVideoROM, video_rom_j_+0x0000 );
    resourceHandler()->add( EfJ6, "a-j6.bin", 0x4000, efVideoROM, video_rom_j_+0x4000 );
    resourceHandler()->add( EfJ7, "a-j7.bin", 0x4000, efVideoROM, video_rom_j_+0x8000 );
    resourceHandler()->add( EfJ8, "a-j8.bin", 0x4000, efVideoROM, video_rom_j_+0xC000 );
    resourceHandler()->add( EfC7, "b-c7.bin", 0x2000, efVideoROM, video_rom_b_+0x0000 );
    resourceHandler()->add( EfD7, "b-d7.bin", 0x2000, efVideoROM, video_rom_b_+0x2000 );
    resourceHandler()->add( EfF7, "b-f7.bin", 0x2000, efVideoROM, video_rom_b_+0x4000 );

    resourceHandler()->addToMachineDriverInfo( info );

    // Setup user interface
    TUserInterface * ui = info->userInterface();
    TUiOption * option;
    TUiOptionGroup * group = ui->addGroup( "DSW A", dtDipSwitch );

    option = group->add( OptCoin1, "Coin 1", 0, 0x03 );
    option->add( S_1Coin1Play, 0x00 );
    option->add( S_1Coin2Play, 0x01 );
    option->add( S_1Coin3Play, 0x02 );
    option->add( S_1Coin6Play, 0x03 );

    option = group->add( OptCoin2, "Coin 2", 0, 0x0C );
    option->add( S_1Coin1Play, 0x00 );
    option->add( S_2Coin1Play, 0x04 );
    option->add( S_1Coin2Play, 0x08 );
    option->add( S_1Coin3Play, 0x0C );

    option = group->add( OptBalls, "Number of balls", 1, 0x30 );
    option->add( "2", 0x30 );
    option->add( "3", 0x00 );
    option->add( "4", 0x10 );
    option->add( "5", 0x20 );

    option = group->add( OptCabinet, S_Cabinet, 0, 0x40 );
    option->add( S_Upright, 0x40 );
    option->add( S_Cocktail, 0x00 );

    option = group->add( OptDemoSounds, "Demo Sounds", 0, 0x80 );
    option->add( S_On, 0x00 );
    option->add( S_Off, 0x80 );

    group = ui->addGroup( "DSW B", dtDipSwitch );

    option = group->add( OptExtendPlay, "Extend play", 0, 0x07 );
    option->add( "70K, 200K",       0x00 );
    option->add( "70K, 200K, 1M",   0x01 );
    option->add( "100K",            0x02 );
    option->add( "100K, 300K",      0x03 );
    option->add( "100K, 300K, 1M",  0x04 );
    option->add( "200K",            0x05 );
    option->add( "200K, 1M",        0x06 );
    option->add( "None",            0x07 );

    // Extra ball setting is the probability of the "extra" lamp lighting
    // up on the main screen
    option = group->add( OptExtraBall, "Extra ball", 0, 0x08 );
    option->add( S_Easy, 0x00 );
    option->add( S_Difficult, 0x08 );

    option = group->add( OptFlippers, "Difficulty for flippers (distance)", 0, 0x30 );
    option->add( S_Easy, 0x00 );
    option->add( S_Normal, 0x10 );
    option->add( S_Challenging, 0x20 );
    option->add( S_Difficult, 0x30 );

    option = group->add( OptOutlanes, "Difficulty in outlanes", 0, 0xC0 );
    option->add( S_Easy, 0x00 );
    option->add( S_Normal, 0x40 );
    option->add( S_Challenging, 0x80 );
    option->add( S_Difficult, 0xC0 );

    // Setup the option handler
    optionHandler()->add( &main_board_.dip_switches_0_, ui, OptCoin1, OptCoin2, OptBalls, OptCabinet, OptDemoSounds );
    optionHandler()->add( &main_board_.dip_switches_1_, ui, OptExtendPlay, OptExtraBall, OptFlippers, OptOutlanes );

    // Ask notification so internal variables are same as GUI
    return true;
}

void PinballAction::reset()
{
    main_board_.reset();
    sound_board_.reset();
}

void PinballAction::run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate )
{
    // Refresh video ROMs if needed
    if( refresh_roms_ ) {
        onVideoROMsChanged();
        refresh_roms_ = false;
    }

    int shake = main_board_.shake_;

    // Run the CPUs for one frame
    TMixerBuffer * mixer_buffer = frame->getMixer()->getBuffer( chMono, samplesPerFrame, 9 );
    int * data_buffer = mixer_buffer->data();
    unsigned steps = 4; // Must be an even number
    unsigned step_size = samplesPerFrame / steps;

    // Make sure we process all samples by setting the proper size for the first step
    unsigned len = samplesPerFrame - (steps-1)*step_size; 

    for( unsigned i=0; i<steps; i++ ) {
        main_board_.setOutputFlipFlop( HaveSoundInterrupt, 0 );

        main_board_.cpu_->run( CpuCyclesPerFrame / steps );

        if( main_board_.output_devices_ & HaveSoundInterrupt ) {
            sound_board_.triggerInterrupt( 0x00 );
        }

        sound_board_.cpu_->run( SoundCpuCyclesPerFrame / steps );

        sound_board_.sound_chip_[0].playSound( data_buffer, len, samplingRate );
        sound_board_.sound_chip_[1].playSound( data_buffer, len, samplingRate );
        sound_board_.sound_chip_[2].playSound( data_buffer, len, samplingRate );

        data_buffer += len;

        if( i == (steps / 2) ) {
            sound_board_.triggerInterrupt( 0x02 );
        }

        len = step_size;
    }

    // Trigger vertical blank interrupts
    if( main_board_.output_devices_ & InterruptEnabled ) {
        main_board_.cpu_->nmi();
    }

    sound_board_.triggerInterrupt( 0x02 );

    // Render the video
    frame->setVideo( renderVideo() );

    // Apply force feedback effects
    if( (shake == 0) && (main_board_.shake_ != 0) ) {
        frame->playForceFeedbackEffect( 0, fxFF_Bump, 0 );
    }
}

void PinballAction::decodeChar( const unsigned char * src, TBitBlock * bb, int ox, int oy, int planes, unsigned plane_size )
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

            bb->setPixel( ox+y, oy+x, color ); // Note that x and y are inverted!
        }

        src++;
    }
}

void PinballAction::decodeSprite( const unsigned char * src, TBitBlock * bb, int ox, int oy )
{
    decodeChar( src,     bb, ox + 0, oy + 8 );
    decodeChar( src + 8, bb, ox + 0, oy );
    decodeChar( src +16, bb, ox + 8, oy + 8 );
    decodeChar( src +24, bb, ox + 8, oy );
}

// Update the internal tables after a ROM has changed
void PinballAction::onVideoROMsChanged()
{
    int i;

    // Decode background character set (4 planes)
    for( i=0; i<2048; i++ ) {
        decodeChar( video_rom_j_ + 8*i, &bg_char_data_, 0, i*8, 4, 0x4000 );
    }

    // Decode foreground character set
    for( i=0; i<1024; i++ ) {
        decodeChar( video_rom_s_ + 8*i, &fg_char_data_, 0, i*8 );

        // Check whether this character is displayable or not (i.e. fully
        // transparent) and flag it accordingly
        int n = 64;
        const unsigned char * s = fg_char_data_.data() + 64*i;

        while( (n > 0) && (*s == 0) ) {
            n--;
            s++;
        }

        fg_char_null_[i] = (n == 0);
    }

    // Decode small sprites set
    for( i=0; i<128; i++ ) {
        decodeSprite( video_rom_b_ + 32*i, &sm_sprite_data_, 0, i*16 );
    }

    // Decode large sprites set
    for( i=0; i<32; i++ ) {
        decodeSprite( 0x1000 + video_rom_b_ + 128*i     , &lg_sprite_data_,  0, i*32 + 16 );
        decodeSprite( 0x1000 + video_rom_b_ + 128*i + 32, &lg_sprite_data_,  0, i*32 );
        decodeSprite( 0x1000 + video_rom_b_ + 128*i + 64, &lg_sprite_data_, 16, i*32 + 16 );
        decodeSprite( 0x1000 + video_rom_b_ + 128*i + 96, &lg_sprite_data_, 16, i*32 );
    }
}

PinballActionMainBoard::PinballActionMainBoard( PinballActionSoundBoard * sound_board )
{
    // Initialize the CPU and the RAM
    cpu_ = new Z80( *this );
    memset( ram_, 0xFF, sizeof(ram_) );

    // Initialize the board variables
    port0_ = 0x00;
    port1_ = 0x00;
    port2_ = 0x00;
    dip_switches_0_ = 0x40;
    dip_switches_1_ = 0x00;
    shake_ = 0;
    sound_board_ = sound_board;

    // Reset the machine
    reset();
}

void PinballActionMainBoard::reset()
{
    cpu_->reset();
    memset( ram_, 0, sizeof(ram_) );
    output_devices_ = 0;
}

void PinballActionMainBoard::setOutputFlipFlop( unsigned char bit, unsigned char value )
{
    if( value ) {
        output_devices_ |= bit;
    }
    else {
        output_devices_ &= ~bit;
    }
}

unsigned char PinballActionMainBoard::readByte( unsigned addr ) 
{
    addr &= 0xFFFF;

    if( addr < sizeof(rom_) )
        return rom_[addr];

    if( addr >= 0xC000 ) {
        if( addr < 0xE600 ) {
            return ram_[addr-0xC000];
        }
        else if( addr == 0xE600 ) {
            // IN0
            return port0_;
        }
        else if( addr == 0xE601 ) {
            // IN1
            return port1_;
        }
        else if( addr == 0xE602 ) {
            // IN2
            return port2_;
        }
        else if( addr == 0xE604 ) {
            // DSW0
            return dip_switches_0_;
        }
        else if( addr == 0xE605 ) {
            // DSW1
            return dip_switches_1_;
        }
        else if( addr == 0xE606 ) {
            // Watchdog reset? The value is read but then thrown away...
            // however the program does so also for E602h in a couple of places...
        }
    }

    return 0xFF;
}

void PinballActionMainBoard::writeByte( unsigned addr, unsigned char b )
{
    addr &= 0xFFFF;

    if( addr >= 0xC000 ) {
        if( addr < 0xE600 ) {
            /*
            C000-CFFF RAM
            D000-D3FF Video RAM
            D400-D7FF Color RAM
            D800-DBFF Background Video RAM
            DC00-DFFF Background Color RAM
            E000-E07F Sprites
            E400-E5FF Palette RAM
            */        
            ram_[addr-0xC000] = b;
        }
        else if( addr == 0xE600 ) {
            // Interrupt enable
            setOutputFlipFlop( InterruptEnabled, b & 0x01 );
        }
        else if( addr == 0xE604 ) {
            // Flip screen
            setOutputFlipFlop( FlipScreen, b & 0x01 );
        }
        else if( addr == 0xE606 ) {
            // Shake (background scroll)
            shake_ = b - 3;
        }
        else if( addr == 0xE800 ) {
            // Command for sound CPU
            sound_board_->command_ = b;
            setOutputFlipFlop( HaveSoundInterrupt, 1 );
        }
    }
}

PinballActionSoundBoard::PinballActionSoundBoard()
{
    sound_chip_[0].setClock( 1500000 );
    sound_chip_[1].setClock( 1500000 );
    sound_chip_[2].setClock( 1500000 );

    cpu_ = new Z80( *this );
    command_ = 0xFF;
    interrupt_pending_ = 0;
}

PinballActionSoundBoard::~PinballActionSoundBoard()
{
    delete cpu_;
}

void PinballActionSoundBoard::reset()
{
    cpu_->reset();
}

void PinballActionSoundBoard::triggerInterrupt( unsigned vector )
{
    if( ! cpu_->interrupt( vector ) ) {
        interrupt_pending_ = (interrupt_pending_ << 8) | (vector+1);
    }
}

void PinballActionSoundBoard::onInterruptsEnabled()
{
    if( interrupt_pending_ ) {
        cpu_->interrupt( (interrupt_pending_ & 0xFF)-1 );
        interrupt_pending_ >>= 8;
    }
}

void PinballActionSoundBoard::run( int interrupt )
{
    if( interrupt ) {
        triggerInterrupt( 0x00 );
    }
}

unsigned char PinballActionSoundBoard::readByte( unsigned addr )
{
    addr &= 0xFFFF;

    if( addr < sizeof(rom_) )
        return rom_[addr];
    else if( (addr >= 0x4000) && (addr < 0x4800) )
        return ram_[addr-0x4000];
    else if( addr == 0x8000 )
        return command_;

    return 0xFF;
}

void PinballActionSoundBoard::writeByte( unsigned addr, unsigned char value )
{
    addr &= 0xFFFF;

    if( (addr >= 0x4000) && (addr < 0x4800) )
        ram_[addr-0x4000] = value;
}

void PinballActionSoundBoard::writePort( unsigned addr, unsigned char value )
{
    addr &= 0xFF;

    switch( addr ) {
    case 0x10: 
    case 0x20: 
    case 0x30:
        sound_chip_[(addr >> 4)-1].writeAddress( value );
        break;
    case 0x11: 
    case 0x21: 
    case 0x31:
        sound_chip_[(addr >> 4)-1].writeData( value );
        break;
    }
}

TBitmapIndexed * PinballAction::renderVideo()
{
    // Update palette
    for( unsigned index=0; index<256; index++ ) {
        unsigned r = (main_board_.ram_[0x2400+index*2] & 0x0F);
        unsigned g = (main_board_.ram_[0x2400+index*2] & 0xF0) >> 4;
        unsigned b = (main_board_.ram_[0x2401+index*2] & 0x0F);
        palette()->setColor( index, TPalette::encodeColor( r*17, g*17, b*17 ) );
    }

    // Draw video
    TBltAdd         bg_char_blitter_f( 0 );
    TBltAddReverse  bg_char_blitter_r( 0 );
    TBltAddSrcZeroTrans         fg_char_blitter_f( 0 );
    TBltAddSrcZeroTransReverse  fg_char_blitter_r( 0 );

    int scroll = (main_board_.output_devices_ & FlipScreen) ? +main_board_.shake_ : -main_board_.shake_;

    int x;
    int y;
    int dx;
    int dy;

    // Draw background
    for( y=0; y<ScreenHeightChars; y++ ) {
        for( x=0; x<ScreenWidthChars; x++ ) {
            unsigned offset = 32*(x+2) + (31-y);
            unsigned attr = main_board_.ram_[0x1C00+offset];
            unsigned code = main_board_.ram_[0x1800+offset] + ((attr & 0x70) << 4);
            unsigned mode;

            if( main_board_.output_devices_ & FlipScreen ) {
                dx = x;
                dy = y;
                mode = 0;
            }
            else {
                dx = 27-x;
                dy = 31-y;
                mode = opFlipX | opFlipY;
            }

            unsigned bg_mode = mode ^ ((attr & 0x80) ? opFlipX : 0);

            TBltAdd * bg_blitter = (bg_mode & opFlipX) ? &bg_char_blitter_r : &bg_char_blitter_f;

            screen()->bits()->copy( dx*8, scroll+dy*8, bg_char_data_, 0, code*8, 8, 8, bg_mode, bg_blitter->color(0x80 + (attr & 0x0F)*16) );
        }
    }

    // Draw sprites
    const unsigned char * spriteram = main_board_.ram_ + 0x2000;

    for( int offset = 0x7C; offset >= 0x00; offset -= 4 ) {
        // Skip this sprite if next is double size
        if( offset > 0x00 && (spriteram[offset-4] & 0x80) )
            continue;

        int sx = spriteram[offset+2];
        int sy = spriteram[offset+3];

        unsigned mode;

        if( main_board_.output_devices_ & FlipScreen ) {
            mode = 0;
            sy = 240 - sy;
            sx = 225 - sx;
            if( spriteram[offset] & 0x80 ) {
                sy -= 16;
                sx -= 16;
            }
        }
        else {
            mode = opFlipX | opFlipY;
            sx = sx - 17;
        }

        if( spriteram[offset+1] & 0x80 ) mode ^= opFlipX;
        if( spriteram[offset+1] & 0x40 ) mode ^= opFlipY;

        TBltAddSrcZeroTrans * blitter = (mode & opFlipX) ? &fg_char_blitter_r : &fg_char_blitter_f;

        if( spriteram[offset] & 0x80 ) {
            screen()->bits()->copy( sx, scroll+sy, lg_sprite_data_, 0, (spriteram[offset] & 0x1F)*32, 32, 32, mode, blitter->color(spriteram[offset+1]*8) );
        }
        else {
            screen()->bits()->copy( sx, scroll+sy, sm_sprite_data_, 0, (spriteram[offset] & 0x7F)*16, 16, 16, mode, blitter->color(spriteram[offset+1]*8) );
        }
    }

    // Draw foreground
    for( y=0; y<ScreenHeightChars; y++ ) {
        for( x=0; x<ScreenWidthChars; x++ ) {
            unsigned offset = 32*(x+2) + (31-y);
            unsigned attr = main_board_.ram_[0x1400+offset];
            unsigned code = main_board_.ram_[0x1000+offset] + ((attr & 0x30) << 4);
            unsigned mode;

            if( fg_char_null_[code] )
                continue;

            if( main_board_.output_devices_ & FlipScreen ) {
                mode = 0;
                dx = x;
                dy = y;
            }
            else {
                dx = 27-x;
                dy = 31-y;
                mode = opFlipX | opFlipY;
            }

            unsigned fg_mode = mode ^ (((attr & 0x40) ? opFlipY : 0) | ((attr & 0x80) ? opFlipX : 0));

            TBltAddSrcZeroTrans * fg_blitter = (fg_mode & opFlipX) ? &fg_char_blitter_r : &fg_char_blitter_f;

            screen()->bits()->copy( dx*8, scroll+dy*8, fg_char_data_, 0, code*8, 8, 8, fg_mode, fg_blitter->color(0x00+(attr & 0x0F)*8) );
        }
    }

    return screen();
}
