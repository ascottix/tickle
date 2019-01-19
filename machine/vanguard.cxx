/*
    Vanguard arcade machine emulator

    Copyright (c) 2011 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include <ase/ase.h>

#include "vanguard.h"

// Hardware info
enum {
    ScreenWidth         = 224,
    ScreenHeight        = 256,
    ScreenColors        = 64,
    VideoFrequency      = 60,
    CpuClock            = 930000,
    SoundClock          = 44100,
    CpuCyclesPerFrame   = CpuClock / VideoFrequency
};

enum { 
    Smp_0 = 0, Smp_1, Smp_2, Smp_3, Smp_4, Smp_5, Smp_6, Smp_7, Smp_8, Smp_9, Smp_A, Smp_B, Smp_C, Smp_D, Smp_E, Smp_F,
    Smp_ShotA, Smp_Bomb,
    Sk4_07, Sk4_08, Sk4_09, Sk4_10, Sk4_12, Sk4_13, Sk4_14, Sk4_15, Sk4_16, Sk5_50, Sk5_51, Sk5_7, Sk5_6, Sk4_51, Sk4_52, Sk4_53, Sk6_07, Sk6_08, Sk6_11,
    Snd_51, Snd_52, Snd_53,
    OptLives, OptCoinCost, OptMultiFire
};

// Machine info
static TMachineInfo VanguardInfo = { 
    "vanguard", "Vanguard", S_SNK, 1981, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TGameRegistrationHandler reg( &VanguardInfo, Vanguard::createInstance );

bool Vanguard::initialize( TMachineDriverInfo * info )
{
    // Declare resources
    resourceHandler()->add( Sk5_7, "sk5_ic7.bin",  0x20, efPalettePROM, palette_prom_ );
    resourceHandler()->add( Sk5_6, "sk5_ic6.bin",  0x20, efPalettePROM, palette_prom_+0x20 );
    
    resourceHandler()->add( Sk4_07, "sk4_ic07.bin", 0x1000, efROM, main_board_->ram_+0x4000 );
    resourceHandler()->add( Sk4_08, "sk4_ic08.bin", 0x1000, efROM, main_board_->ram_+0x5000 );
    resourceHandler()->add( Sk4_09, "sk4_ic09.bin", 0x1000, efROM, main_board_->ram_+0x6000 );
    resourceHandler()->add( Sk4_10, "sk4_ic10.bin", 0x1000, efROM, main_board_->ram_+0x7000 );
    resourceHandler()->add( Sk4_13, "sk4_ic13.bin", 0x1000, efROM, main_board_->ram_+0x8000 );
    resourceHandler()->add( Sk4_14, "sk4_ic14.bin", 0x1000, efROM, main_board_->ram_+0x9000 );
    resourceHandler()->add( Sk4_15, "sk4_ic15.bin", 0x1000, efROM, main_board_->ram_+0xA000 );
    resourceHandler()->add( Sk4_16, "sk4_ic16.bin", 0x1000, efROM, main_board_->ram_+0xB000 );
    
    resourceHandler()->add( Sk5_50, "sk5_ic50.bin", 0x800, efVideoROM, main_board_->ram_+0x2000 );
    resourceHandler()->add( Sk5_51, "sk5_ic51.bin", 0x800, efVideoROM, main_board_->ram_+0x2800 );

    resourceHandler()->add( Sk4_51, "sk4_ic51.bin", 0x800, efSoundPROM, main_board_->sound_rom_ );
    resourceHandler()->add( Sk4_52, "sk4_ic52.bin", 0x800, efSoundPROM, main_board_->sound_rom_+0x800 );

    resourceHandler()->add( Sk6_07, "sk6_ic07.bin", 0x800, efSoundPROM, main_board_->speech_rom_ );
    resourceHandler()->add( Sk6_08, "sk6_ic08.bin", 0x800, efSoundPROM, main_board_->speech_rom_+0x0800 );
    resourceHandler()->add( Sk6_11, "sk6_ic11.bin", 0x800, efSoundPROM, main_board_->speech_rom_+0x1000 );
    
    resourceHandler()->add( Smp_0, "vg_voi-0.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_1, "vg_voi-1.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_2, "vg_voi-2.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_3, "vg_voi-3.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_4, "vg_voi-4.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_5, "vg_voi-5.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_6, "vg_voi-6.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_7, "vg_voi-7.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_8, "vg_voi-8.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_9, "vg_voi-9.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_A, "vg_voi-a.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_B, "vg_voi-b.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_C, "vg_voi-c.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_D, "vg_voi-d.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_E, "vg_voi-e.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_F, "vg_voi-f.wav", 0x0000, efSample, 0 );
    
    resourceHandler()->add( Smp_ShotA, "fire.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_Bomb, "explsion.wav", 0x0000, efSample, 0 );
    
    resourceHandler()->addToMachineDriverInfo( info );

    // Setup user interface
    TUserInterface * ui = info->userInterface();
    
    TUiOptionGroup * group = ui->addGroup( S_DSW_1, dtDipSwitch );

    TUiOption * option = group->add( OptLives, S_Lives, 0, 0x30 );
    option->add( "3", 0x00 );
    option->add( "4", 0x10 );
    option->add( "5", 0x20 );

    option = group->add( OptCoinCost, S_Coin, 0, 0x40 );
    option->add( S_1Coin1Play, 0x00 );
    option->add( S_2Coin1Play, 0x40 );
    
    optionHandler()->add( &main_board_->dsw0_, ui, OptLives, OptCoinCost );

    group = ui->addGroup( "Trainer", dtCheat );

    option = group->add( OptMultiFire, "Multi-fire", 0, 0x30 );
    option->add( S_Off, 0x00 );
    option->add( S_On,  0x01 );
    
    optionHandler()->add( &main_board_->cheat_options_, ui, OptMultiFire );
    
    // Ask notification so internal variables are same as GUI
    return true;
}

bool Vanguard::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    int result = 0;
    
    if( id >= Smp_0 && id <= Smp_F ) {
        main_board_->speech_samples_[ id-Smp_0 ].setSample( copySample(buf,len) );
    }
    else if( id == Smp_ShotA ) {
        main_board_->sample_shot_a_.setSample( copySample(buf,len) );
    }
    else if( id == Smp_Bomb ) {
        main_board_->sample_bomb_.setSample( copySample(buf,len) );
    }
    else {
        // Need to refresh some internal tables if the video/color ROMs change
        refresh_roms_ |= (id == Sk5_50) || (id == Sk5_51);

        result = resourceHandler()->handle( id, buf, len );
       
        // Wait until the IC13 ROM has been loaded before resetting the CPU (it contains the reset vector)
        if( id == Sk4_13 ) {        
            main_board_->reset();
        }
    }

    return 0 == result;
}

Vanguard::Vanguard( VanguardBoard * board ) :
    main_board_( board ),
    back_char_data_( 8, 8*256 ),
    fore_char_data_( 8, 256*8 )
{
    refresh_roms_ = false;
    back_color_ = (unsigned) -1;
    
    createScreen( ScreenWidth, ScreenHeight, ScreenColors );

    setJoystickHandler( 0, new TJoystickToPortHandler(idJoyP1Joystick1, ptNormal, jm4Way, &board->port0_, 0x10204080) );
    setJoystickHandler( 1, new TJoystickToPortHandler(idJoyP2Joystick1, ptNormal, jm4Way, &board->port1_, 0x10204080) );

    eventHandler()->add( idCoinSlot1,        ptNormal, &board->coin_slot_, 0x02 );
    eventHandler()->add( idCoinSlot2,        ptNormal, &board->coin_slot_, 0x01 );
    eventHandler()->add( idKeyStartPlayer1,  ptNormal, &board->port2_, 0x80 );
    eventHandler()->add( idKeyStartPlayer2,  ptNormal, &board->port2_, 0x40 );
    
    eventHandler()->add( idKeyP1Action1,     ptNormal, &board->port0_, 0x04 ); // Forward
    eventHandler()->add( idKeyP1Action2,     ptNormal, &board->port0_, 0x08 ); // Backward
    eventHandler()->add( idKeyP1Action3,     ptNormal, &board->port0_, 0x01 ); // Down
    eventHandler()->add( idKeyP1Action4,     ptNormal, &board->port0_, 0x02 ); // Up

    eventHandler()->add( idKeyP2Action1,     ptNormal, &board->port1_, 0x08 );
    eventHandler()->add( idKeyP2Action2,     ptNormal, &board->port1_, 0x04 );
    eventHandler()->add( idKeyP2Action3,     ptNormal, &board->port1_, 0x01 );
    eventHandler()->add( idKeyP2Action4,     ptNormal, &board->port1_, 0x02 );
    
    registerDriver( VanguardInfo );
}

Vanguard::~Vanguard()
{
    delete main_board_;
}

void Vanguard::reset()
{
    main_board_->reset();
}

void Vanguard::run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate )
{
    // Update if ROM changed since last frame
    if( refresh_roms_ ) {
        onVideoROMsChanged();
        refresh_roms_ = false;
    }

    // Run the game for one frame
    main_board_->run();

    frame->setVideo( renderVideo(), false );

    // Play the explosion sound
    if( main_board_->sn_bomb_.isOutputEnabled() ) {
        TMixerBuffer * mixer_buffer = frame->getMixer()->getBuffer( chMono, samplesPerFrame, 1 );
        
        main_board_->sn_bomb_.playSound( mixer_buffer->data(), samplesPerFrame, samplingRate );
    }
    
    // Play the "shot B" sound
    if( main_board_->sn_shot_b_.isOutputEnabled() ) {
        TMixerBuffer * mixer_buffer = frame->getMixer()->getBuffer( chMono, samplesPerFrame, 1 );
        
        main_board_->sn_shot_b_.playSound( mixer_buffer->data(), samplesPerFrame, samplingRate );
    }
    
    // Play the music (wait one second to allow system initialization)
    if( main_board_->frame_counter_ >= VideoFrequency ) {
        TMixerBuffer * mixer_buffer = frame->getMixer()->getBuffer( chMono, samplesPerFrame, 0 );

        main_board_->sound_board_.play( mixer_buffer, samplesPerFrame, samplingRate );
    }
    
    main_board_->hd38880_.play( frame, samplesPerFrame, samplingRate );
    
    // Play the samples for the sound circuits that have not been emulated
    main_board_->sample_shot_a_.mix( frame->getMixer(), chMono, samplesPerFrame, samplingRate );
    main_board_->sample_bomb_.mix( frame->getMixer(), chMono, samplesPerFrame, samplingRate );
}

void Vanguard::decodeCharSet( unsigned char * b_src, unsigned char * b_dst, unsigned plane_offset )
{
    for( int i=0; i<256; i++ ) {
        unsigned char * src = b_src +  8*i;
        unsigned char * dst = b_dst + 64*i;
        
        for( int y=0; y<8; y++ ) {
            for( int x=0; x<8; x++ ) {
                int o = 1 << y;
                int b1 = src[x] & o ? 1 : 0;
                int b0 = src[x+plane_offset] & o ? 1 : 0;
                dst[ 8*(7-y) + 7-x ] = b0+b1*2;
            }
        }
    }
}

// Update the internal tables after a ROM has changed
void Vanguard::onVideoROMsChanged()
{
    int i;
    
    // Decode palette
    for( i=0; i<64; i++ ) {
        palette_rgb_[i] = TPalette::decodeByte(palette_prom_[i]);
        palette()->setColor( i, palette_rgb_[i] );
    }

    // Decode character set
    decodeCharSet( main_board_->ram_ + 0x2000, (unsigned char *) back_char_data_.data(), 0x800 );
}

TBitmapIndexed * Vanguard::renderVideo( TBitmapIndexed * screen, unsigned char * ram, TBitBlock & back_char_data, TBitBlock & fore_char_data, unsigned back_color, int scroll_x, int scroll_y )
{
    // Render background characters
    unsigned char * video_ram = ram + 0x800;
    unsigned char * color_ram = ram + 0xC00;
    
    TBltAdd bg_blitter_f(back_color);
    TBltAdd * bg_blitter = &bg_blitter_f;
    
    for( int cx=0; cx<32; cx++ ) {
        for( int cy=0; cy<32; cy++ ) {
            // Get the offset of this location in the video RAM
            unsigned offset = 32*(31-cx) + cy;
            
            unsigned char v = video_ram[offset];
            unsigned char c = (color_ram[offset] & 0x38) >> 3;
            
            int x = cx*8 + scroll_x;
            int y = cy*8 - scroll_y;
            
            if( x <    0 ) x += 256;
            if( x >= 256 ) x -= 256;
            if( y <    0 ) y += 256;
            if( y >= 256 ) y -= 256;
            
            x -= 32;
            
            screen->bits()->copy( x, y, back_char_data, 0, 8*v, 8, 8, opAdd, bg_blitter->color(c*4+0x20) );
        }
    }
    
    // Render foreground characters (sprites)
    video_ram = ram + 0x400;
    
    TBltAddSrcZeroTrans fg_blitter_f( 0 );
    TBltAddSrcZeroTrans * fg_blitter = &fg_blitter_f;
    
    for( int cx=0; cx<32; cx++ ) {
        for( int cy=0; cy<32; cy++ ) {
            // Get the offset of this location in the video RAM
            unsigned offset = 32*(31-cx) + cy;
            
            unsigned char v = video_ram[offset];
            unsigned char c = color_ram[offset] & 0x07;
            
            screen->bits()->copy( (cx-4)*8, cy*8, fore_char_data, 0, 8*v, 8, 8, opAdd, fg_blitter->color(c*4) );
        }
    }
    
    return screen;
}

TBitmapIndexed * Vanguard::renderVideo()
{
    // A little trick to save time, we should actually fill the screen with the background color
    // and then render the background characters as "sprites" (with transparency)
    for( int i=0x20; i<0x40; i+=4 ) {
        palette()->setColor( i, palette_rgb_[ main_board_->back_color_ * 4 + 0x20 ] );
    }

    decodeCharSet( main_board_->ram_ + 0x1000, (unsigned char *) fore_char_data_.data(), 0x800 );

    return renderVideo( screen(), main_board_->ram_, back_char_data_, fore_char_data_, main_board_->back_color_, main_board_->scroll_x_, main_board_->scroll_y_ );
}

static const unsigned SpeechAddressTable[16] = {
    0x04000,    // Ha ha ha...
    0x04325,    // Bon voyage
    0x044a2,    // Let's attack
    0x045b7,    // Stick zone
    0x046ee,    // Stripe zone
    0x04838,    // Bleak zone
    0x04984,    // Rainbow zone
    0x04b01,    // Last zone
    0x04c38,    // Congratulations
    0x04de6,    // Good luck
    0x04f43,    // Ho ho ho
    0x05048,    // Keep quiet
    0x05160,    // Keep your way
    0x05289,    // Very good
    0x0539e,    // Be careful
    0x054ce     // I'm hungry
};

VanguardBoard::VanguardBoard() :
    sound_board_( sound_rom_, 2 )
{
    cpu_ = new N6502( *this );
    
    memset( ram_, 0, sizeof(ram_) );

    port0_ = 0;
    port1_ = 0;
    port2_ = 0;
    coin_slot_ = 0;
    dsw0_ = 0xFF;
    cheat_options_ = 0;
    cheat_multi_fire_ = 0x01;
    back_color_ = 0;
    scroll_x_ = 0;
    scroll_y_ = 0;
    frame_counter_ = 0;
    o_port_3100_ = 0;

    hd38880_.setSamples( 16, SpeechAddressTable, speech_samples_ );
    
    // Music channels
    sound_board_.channel(0)->setOneShot( true );

    // Bomb (part of a more complex circuit)
    sn_bomb_.setNoiseClock( Kilo(470) ); // 470K
    sn_bomb_.setNoiseFilter( Mega(1.5), Pico(220) ); // 1.5M, 220p
    sn_bomb_.setAmplifier( Kilo(4.7), Kilo(47) ); // 4.7K, 47K
    sn_bomb_.setMixer( snMixer_Noise );
    sn_bomb_.setEnvelope( snEnv_VCO ); // Pin 1 + Pin 28 (not drawn in the schematics)

    // Shot B
    sn_shot_b_.setNoiseClock( Kilo(10) ); // 10K
    sn_shot_b_.setNoiseFilter( Kilo(30), 0 ); // 30K but no capacitor (no filter)
    sn_shot_b_.setAmplifier( Kilo(4.7), Kilo(47) ); // 4.7K, 47K
    sn_shot_b_.setMixer( snMixer_Noise );
    sn_shot_b_.setEnvelope( snEnv_MixerOnly ); // Pin 28 only
}

VanguardBoard::~VanguardBoard()
{
    delete cpu_;
}

void VanguardBoard::reset()
{
    cpu_->reset();
}

void VanguardBoard::run()
{
    frame_counter_++;
    
    // Run
    cpu_->run( CpuCyclesPerFrame );
    
    unsigned char o_port2 = port2_ & 3;
    port2_ = (port2_ & ~3) | (coin_slot_ & 3);
    unsigned char n_port2 = port2_ & 3;
    
    // Force a CPU interrupt once per frame
    if( n_port2 && n_port2 != o_port2 ) {
        cpu_->interrupt( N6502::Int_NMI ); // Coin slot triggers a NMI
    }
    else {
        cpu_->interrupt( N6502::Int_IRQ );
    }
}

unsigned char VanguardBoard::readByte( unsigned addr ) 
{
    addr &= 0xFFFF;

    // Rapidly fire in all directions (one direction at a time)
    if( addr == 0x3104 && (cheat_options_ & 0x01) && port0_ & 0x0F ) {
        cheat_multi_fire_ <<= 1;
        if( cheat_multi_fire_ >= 0x10 ) cheat_multi_fire_ = 0x01;
        
        return (port0_ & ~0x0F) | cheat_multi_fire_;
    }
    
    // Check to see if address is memory mapped port
    switch( addr ) {
    case 0x3104: 
        // Player 1 control
        return port0_;
    case 0x3105: 
        // Player 2 control
        return port1_;
    case 0x3106:
        // DIP switches
        return dsw0_;
    case 0x3107: 
        // Coin slots and start game buttons
        return port2_;
    }
    
    if( addr >= 0xFFFA ) {
        addr -= 0x7000; // CPU reset and interrupt vectors
    }

    return ram_[addr];
}

void VanguardBoard::writeByte( unsigned addr, unsigned char b )
{
    addr &= 0xFFFF;
    
    if( addr < 0x2000 ) {
        // 0x0000-0x03FF: RAM
        // 0x0400-0x07FF: Video RAM #1
        // 0x0800-0x0BFF: Video RAM #2
        // 0x0C00-0x0FFF: Color RAM
        // 0x1000-0x1FFF: Character RAM
        ram_[addr] = b;
    }
    
    switch( addr ) {
        // Memory mapped ports
        case 0x3100:
            // Sound port 0
            //
            // Bit 0 = music A10
            // Bit 1 = music A9
            // Bit 2 = music A8
            // Bit 3 = LS05 port 1
            // Bit 4 = LS04 port 2
            // Bit 5 = shot A
            // Bit 6 = shot B
            // Bit 7 = bomb
            sound_board_.channel(0)->setOffset( ((int)(b & 0x07)) << 8 );
            
            if( b & 0x08 ) {
                sound_board_.channel(0)->setMuted(true);
            }
            if( b & 0x10 ) {
                sound_board_.channel(0)->setMuted(false);
            }
            
            sn_shot_b_.enableOutput( (b & 0x40) !=0 );
            
            if( b & 0x20 ) {
                if( (o_port_3100_ & 0x20) == 0 ) sample_shot_a_.restart();
                sample_shot_a_.play();
            }
            
            if( b & 0x80 ) {
                if( (o_port_3100_ & 0x80) == 0 ) sample_bomb_.restart();
                sample_bomb_.play();
            }
            
            sn_bomb_.enableOutput( (b & 0x80) != 0 );
            
            o_port_3100_ = b;
            break;
        case 0x3101:
            // Sound port 1
            //
            // Bit 0 = music A10
            // Bit 1 = music A9
            // Bit 2 = music A8
            // Bit 3 = LS04 port 3
            // Bit 4 = EXTP A (HD38880 external pitch control A)
            // Bit 5 = EXTP B (HD38880 external pitch control B)
            sound_board_.channel(1)->setOffset( (b & 0x07) << 8 );
            sound_board_.channel(1)->setMuted( (b & 0x08) == 0 );
            break;
        case 0x3102:
            // Sound port 2
            //
            // Bit 0-3 = sound 0 waveform
            // Bit 4-7 = sound 1 waveform
            sound_board_.channel(0)->setWaveform( b & 0x0F );
            sound_board_.channel(1)->setWaveform( b >> 4 );
            break;
        case 0x3103:
            // Screen control
            back_color_ = b & 0x07;
            break;
        case 0x3200:
            // Y scroll register
            scroll_y_ = (int) (signed char) b;
            break;
        case 0x3300:
            // X scroll register
            scroll_x_ = (int) (signed char) b;
            break;
        case 0x3400:
            hd38880_.write( b );
            break;
    }
}
