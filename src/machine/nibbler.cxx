/*
    Nibbler arcade machine emulator

    Copyright (c) 2011 Alessandro Scotti
*/
#include "nibbler.h"

// Hardware info
enum {
    ScreenWidth         = 224,
    ScreenHeight        = 256,
    ScreenColors        = 64,
    VideoFrequency      = 60,
    CpuClock            = 930000,
    CpuCyclesPerFrame   = CpuClock / VideoFrequency
};

enum { 
    Smp_ShotA, Smp_Bomb,
    Sk4_07, Sk4_08, Sk4_09, Sk4_10, Sk4_12, Sk4_14, Sk4_15, Sk4_16, Sk4_17,
    Sk5_7, Sk5_6,
    Sk5_50, Sk5_51,
    Snd_51, Snd_52, Snd_53,
    OptLives, OptDiagnostics, OptCoinage
};

// Machine info
static TMachineInfo NibblerInfo = { 
    "nibbler", "Nibbler", S_RockOla, 1982, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TGameRegistrationHandler reg( &NibblerInfo, Nibbler::createInstance );

const AFloat HitFilterGain = 16e+02;
const AFloat HitStreamGain = 4000;
const int HitVolume = 51;

bool Nibbler::initialize( TMachineDriverInfo * info )
{
    resourceHandler()->add( Sk5_7, "g-0708-05.ic7",  0x20, efPalettePROM, palette_prom_ );
    resourceHandler()->add( Sk5_6, "g-0708-04.ic6",  0x20, efPalettePROM, palette_prom_+0x20 );
    
    resourceHandler()->add( Sk4_12, "g-0960-52.ic12", 0x1000, efROM, main_board_->ram_+0x3000 );
    resourceHandler()->add( Sk4_07, "g-0960-48.ic7",  0x1000, efROM, main_board_->ram_+0x4000 );
    resourceHandler()->add( Sk4_08, "g-0960-49.ic8",  0x1000, efROM, main_board_->ram_+0x5000 );
    resourceHandler()->add( Sk4_09, "g-0960-50.ic9",  0x1000, efROM, main_board_->ram_+0x6000 );
    resourceHandler()->add( Sk4_10, "g-0960-51.ic10", 0x1000, efROM, main_board_->ram_+0x7000 );
    resourceHandler()->add( Sk4_14, "g-0960-53.ic14", 0x1000, efROM, main_board_->ram_+0x8000 );
    resourceHandler()->add( Sk4_15, "g-0960-54.ic15", 0x1000, efROM, main_board_->ram_+0x9000 );
    resourceHandler()->add( Sk4_16, "g-0960-55.ic16", 0x1000, efROM, main_board_->ram_+0xA000 );
    resourceHandler()->add( Sk4_17, "g-0960-56.ic17", 0x1000, efROM, main_board_->ram_+0xB000 );
    
    resourceHandler()->add( Sk5_50, "g-0960-57.ic50", 0x1000, efVideoROM, main_board_->ram_+0xC000 );
    resourceHandler()->add( Sk5_51, "g-0960-58.ic51", 0x1000, efVideoROM, main_board_->ram_+0xD000 );
    
    resourceHandler()->add( Snd_51, "g-0959-43.ic51", 0x800, efROM, main_board_->sound_rom_ );
    resourceHandler()->add( Snd_52, "g-0959-44.ic52", 0x800, efROM, main_board_->sound_rom_+0x0800 );
    resourceHandler()->add( Snd_53, "g-0959-45.ic53", 0x800, efROM, main_board_->sound_rom_+0x1000 );
    
    resourceHandler()->addToMachineDriverInfo( info );
    
    // Setup user interface
    TUserInterface * ui = info->userInterface();
    
    TUiOptionGroup * group = ui->addGroup( S_DSW_1, dtDipSwitch );
    
    TUiOption * option = group->add( OptLives, S_Lives, 0, 0x03 );
    option->add( "3", 0x00 );
    option->add( "4", 0x01 );
    option->add( "5", 0x02 );
    option->add( "6", 0x03 );

    option = group->add( OptDiagnostics, "Diagnostics", 0, 0x10 );
    option->add( S_Off, 0x00 );
    option->add( S_On,  0x10 );
    
    option = group->add( OptCoinage, "Coinage", 0, 0xE0 );
    option->add( S_1Coin1Play,  0x00 );
    option->add( S_2Coin1Play,  0x40 );
    option->add( S_2Coin3Play,  0x80 );
    option->add( S_4Coin3Play,  0xC0 );
    option->add( S_FreePlay,    0x20 );
    
    optionHandler()->add( &main_board_->dsw0_, ui, OptLives, OptDiagnostics, OptCoinage );
    
    // Ask notification so internal variables are same as GUI
    return true;
}

bool Nibbler::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    int result = 0;

    // Need to refresh some internal tables if the video/color ROMs change
    refresh_roms_ |= (id == Sk5_50) || (id == Sk5_51);

    result = resourceHandler()->handle( id, buf, len );
   
    // Wait until the IC14 ROM has been loaded before resetting the CPU (it contains the reset vector)
    if( id == Sk4_14 ) {        
        main_board_->reset();
    }

    return 0 == result;
}

Nibbler::Nibbler( NibblerBoard * board ) :
    main_board_( board ),
    fore_char_data( 8, 8*256 )
{
    refresh_roms_ = false;
    back_color_ = (unsigned) -1;
    char_data_[0] = new TBitBlock( 8, 8*256 );
    char_data_[1] = new TBitBlock( 8, 8*256 );
    
    createScreen( ScreenWidth, ScreenHeight, ScreenColors );

    setJoystickHandler( 0, new TJoystickToPortHandler(idJoyP1Joystick1, ptNormal, jm4Way, &board->port0_, 0x10204080) );
    setJoystickHandler( 1, new TJoystickToPortHandler(idJoyP2Joystick1, ptNormal, jm4Way, &board->port1_, 0x10204080) );

    eventHandler()->add( idCoinSlot1,        ptNormal, &board->coin_slot_, 0x02 );
    eventHandler()->add( idCoinSlot2,        ptNormal, &board->coin_slot_, 0x01 );
    eventHandler()->add( idKeyStartPlayer1,  ptNormal, &board->port2_, 0x80 );
    eventHandler()->add( idKeyStartPlayer2,  ptNormal, &board->port2_, 0x40 );
    
    registerDriver( NibblerInfo );
}

Nibbler::~Nibbler()
{
    delete main_board_;
    delete char_data_[0];
    delete char_data_[1];
}

void Nibbler::reset()
{
    main_board_->reset();
}

void Nibbler::run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate )
{
    // Update if ROM changed since last frame
    if( refresh_roms_ ) {
        onVideoROMsChanged();
        refresh_roms_ = false;
    }

    // Run the game for one frame
    main_board_->run();

    frame->setVideo( renderVideo(), false );

    // Play the music
    TMixerBuffer * mixer_buffer = frame->getMixer()->getBuffer( chMono, samplesPerFrame, 0 );

    main_board_->sound_board_.play( mixer_buffer, samplesPerFrame, samplingRate );
    
    // Explosion
    main_board_->a_rc_filter_3_->resetStream();
    main_board_->a_rc_filter_3_->updateTo( samplesPerFrame );
    main_board_->a_rc_filter_3_->mixStream( mixer_buffer->data(), HitStreamGain, 0, HitVolume ); // Noise must be clipped
    
    mixer_buffer->addVoices( 0 );
}

// Update the internal tables after a ROM has changed
void Nibbler::onVideoROMsChanged()
{
    int i;
    
    // Assign palette
    for( i=0; i<64; i++ ) {
        palette_rgb_[i] = TPalette::decodeByte(palette_prom_[i]);
        palette()->setColor( i, palette_rgb_[i] );
    }

    // Decode character set
    Vanguard::decodeCharSet( main_board_->ram_ + 0xC000, (unsigned char *) char_data_[1]->data(), 0x1000 );
    Vanguard::decodeCharSet( main_board_->ram_ + 0xC800, (unsigned char *) char_data_[0]->data(), 0x1000 );
}

TBitmapIndexed * Nibbler::renderVideo()
{
    for( int i=0x20; i<0x40; i+=4 ) {
        palette()->setColor( i, palette_rgb_[ main_board_->back_color_ * 4 + 0x20 ] );
    }
    
    Vanguard::decodeCharSet( main_board_->ram_ + 0x1000, (unsigned char *) fore_char_data.data(), 0x800 );
    
    return Vanguard::renderVideo( screen(), main_board_->ram_, *char_data_[main_board_->char_bank_], fore_char_data, main_board_->back_color_, main_board_->scroll_x_, main_board_->scroll_y_ );
}

NibblerBoard::NibblerBoard() :
    sound_board_( sound_rom_, 3 )
{
    cpu_ = new N6502( *this );
    
    memset( ram_, 0, sizeof(ram_) );

    port0_ = 0;
    port1_ = 0;
    port2_ = 0;
    coin_slot_ = 0;
    dsw0_ = 0;
    back_color_ = 0;
    char_bank_ = 0;
    scroll_x_ = 0;
    scroll_y_ = 0;
    frame_counter_ = 0;
    
    // In the Nibbler sound board a SN76477 is used to generate the noise signal. The noise frequency
    // is controlled by a 470K resistor connected to pin 4, which corresponds to approx 3082 Hz (see MAME driver for SN76477)
    a_noise_ = new AWhiteNoise( SN76477::getNoiseFreqFromRes(Kilo(470)) ); // R50 connected to pin 4 of 76477
    a_noise_->setOutput( 3.4, 0.2 );
    a_noise_rc_filter_ = new ALowPassRCFilter( *a_noise_, Kilo(6.8), Micro(0.033) ); // R37, C25
    a_hit_latch_ = new ALatch; // Hit (explosion)
    a_hit_diode_ = new AClipperLo( *a_hit_latch_, 0.7 ); // D3
    a_hit_ = new ACapacitorWithSwitch( *a_hit_diode_, *a_noise_rc_filter_, Kilo(1), Kilo(43), Micro(1) ); // R15, R52+R53, C24
    a_hit_filter_ = new AActiveBandPassFilter( *a_hit_, Kilo(10), Kilo(33), Kilo(470), Micro(0.01), Micro(0.01) ); // R52, R53, R48, C42, C41, LM324
    a_hit_filter_->setGain( HitFilterGain );
    a_rc_filter_1_ = new ALowPassRCFilter( *a_hit_filter_, Kilo(22), Micro(0.01) ); // R57, C45
    a_rc_filter_2_ = new ALowPassRCFilter( *a_rc_filter_1_, Kilo(22), Micro(2.2) ); // R58, C46
    a_rc_filter_3_ = new ALowPassRCFilter( *a_rc_filter_2_, Kilo(22), Micro(0.001) ); // R59, C51
}

NibblerBoard::~NibblerBoard()
{
    delete cpu_;

    delete a_noise_;
    delete a_noise_rc_filter_;
    delete a_hit_latch_;
    delete a_hit_diode_;
    delete a_hit_;
    delete a_hit_filter_;
    delete a_rc_filter_1_;
    delete a_rc_filter_2_;
    delete a_rc_filter_3_;
}

void NibblerBoard::reset()
{
    cpu_->reset();
}

void NibblerBoard::run()
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

unsigned char NibblerBoard::readByte( unsigned addr ) 
{
    addr &= 0xFFFF;

    // Check to see if address is memory mapped port
    switch( addr ) {
        case 0x2104: 
            // Player 1 control
            return port0_;
        case 0x2105: 
            // Player 2 control
            return port1_;
        case 0x2106:
            // DIP switches
            return dsw0_;
        case 0x2107: 
            // Coin slots and start game buttons
            return port2_;
    }
    
    if( addr >= 0xF000 ) {
        addr -= 0x7000; // CPU reset and interrupt vectors
    }
    
    return ram_[addr];
}

void NibblerBoard::writeByte( unsigned addr, unsigned char b )
{
    addr &= 0xFFFF;
    
    if( addr < 0x2000 ) {
        ram_[addr] = b;
    }
    
    switch( addr ) {
        // Memory mapped ports
        case 0x2100:
            // Sound port 0
            sound_board_.channel(0)->setOffset( ((int)(b & 0x07)) << 8 );
            sound_board_.channel(0)->setMuted( (b & 0x08) == 0 );
            sound_board_.channel(2)->setMuted( (b & 0x10) == 0 );
            
            a_hit_latch_->setValue( b & 0x80 ? 3.2 : 0.3 );
            break;
        case 0x2101:
            // Sound port 1
            sound_board_.channel(1)->setOffset( (b & 0x07) << 8 );
            sound_board_.channel(1)->setMuted( (b & 0x08) == 0 );
            break;
        case 0x2102:
            // Sound port 2
            sound_board_.channel(0)->setWaveform( b & 0x0F );
            sound_board_.channel(1)->setWaveform( b >> 4 );
            break;
        case 0x2103:
            // Screen control and extra music channel
            back_color_ = b & 0x07;
            char_bank_ = (b & 0x08) >> 3;
            sound_board_.channel(2)->setOffset( ((int)(b & 0x70)) << 4 );
            break;
        case 0x2200:
            // Y scroll register
            scroll_y_ = (int) (signed char) b;
            break;
        case 0x2300:
            // X scroll register
            scroll_x_ = (int) (signed char) b;
            break;
    }
}
