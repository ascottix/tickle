/*
    Space Invaders arcade machine emulator

    Copyright (c) 1997-2010,2011 Alessandro Scotti
*/
#include <math.h>

#include "invaders.h"

enum {
    ScreenWidth     = 224,
    ScreenHeight    = 256,
    ScreenColors    = 8,
    VideoFrequency  = 60,
    CpuCyclesPerInterrupt = 2000000 / (2*VideoFrequency) // 2MHz CPU, 60 FPS, 2 interrupts per frame
};

// External files id (note that declaration order is important here!) and options
enum { 
    EfUfo, EfShot, EfBomb, EfTargetHit, EfStep1, EfStep2, EfStep3, EfStep4, EfUfoHit, EfExtendPlay,
    Ef1, Ef2, Ef3, Ef4, Ef5, Ef6, EfPROM1, EfPROM2,
    OptLives, OptBonus, OptCoinInfo, OptScreenType, OptRedScreen, OptAnalogSounds
};

// Machine info
static TMachineInfo SpaceInvadersInfo = { 
    "invaders", "Space Invaders", S_Taito, 1979, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo SpaceInvadersPartIIInfo = { 
    "invadpt2", "Space Invaders Part II", S_Taito, 1979, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo SpaceInvadersDeluxeInfo = { 
    "invaddlx", "Space Invaders Deluxe", S_Midway, 1979, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo LunarRescueInfo = { 
    "lrescue", "Lunar Rescue", S_Taito, 1979, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo SpaceAttackIIInfo = { 
    "spaceat2", "Space Attack II", "Zenitone-Microsec LTD", 1980, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo OzmaWarsInfo = { 
    "ozmawars", "Ozma Wars", S_SNK, 1979, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo RollingCrashInfo = { 
    "rollingc", "Rolling Crash / Moon Base", S_Nichibutsu, 1979, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TGameRegistrationHandler reg1( &SpaceInvadersInfo,       SpaceInvaders::createInstance );
static TGameRegistrationHandler reg2( &SpaceInvadersPartIIInfo, SpaceInvadersPartII::createInstance );
static TGameRegistrationHandler reg3( &LunarRescueInfo,         LunarRescue::createInstance );
static TGameRegistrationHandler reg4( &SpaceAttackIIInfo,       SpaceAttackII::createInstance );
static TGameRegistrationHandler reg5( &OzmaWarsInfo,            OzmaWars::createInstance );
static TGameRegistrationHandler reg6( &SpaceInvadersDeluxeInfo, SpaceInvadersDeluxe::createInstance );
static TGameRegistrationHandler reg7( &RollingCrashInfo,        RollingCrash::createInstance );

bool SpaceInvaders::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    bool result = false;

    if( (id >= EfUfo) && (id <= EfExtendPlay) ) {
        samples_[id-EfUfo].setSample( copySample(buf,len) );
        result = true;
    }
    else {
        result = 0 == resourceHandler()->handle( id, buf, len );
    }

    return result;
}

bool SpaceInvaders::initialize( TMachineDriverInfo * info )
{
    // Declare resources
    resourceHandler()->add( Ef1, "invaders.h", 0x0800, efROM, main_board_->ram_+0x0000 );
    resourceHandler()->add( Ef2, "invaders.g", 0x0800, efROM, main_board_->ram_+0x0800 );
    resourceHandler()->add( Ef3, "invaders.f", 0x0800, efROM, main_board_->ram_+0x1000 );
    resourceHandler()->add( Ef4, "invaders.e", 0x0800, efROM, main_board_->ram_+0x1800 );

    resourceHandler()->addToMachineDriverInfo( info );

    // Setup user interface
    TUserInterface * ui = info->userInterface();
    TUiOption * option;
    TUiOptionGroup * group = ui->addGroup( S_DSW_1, dtDipSwitch );

    option = group->add( OptLives, "Ships per game", 0, 0x03 );
    option->add( "3", 0x00 );
    option->add( "4", 0x01 );
    option->add( "5", 0x02 );
    option->add( "6", 0x03 );

    option = group->add( OptBonus, "Bonus ship at", 1, 0x08 );
    option->add( "1000", 0x08 );
    option->add( "1500", 0x00 );

    option = group->add( OptCoinInfo, "Coin info", 0, 0x80 );
    option->add( S_On, 0x00 );
    option->add( S_Off, 0x80 );

    group = ui->addGroup( "Options", dtDriverOption );

    option = group->add( OptScreenType, "Hardware type", 4, 0x07 );
    option->add( "Monochrome (white)", 0x00 );
    option->add( "Monochrome (cyan)", 0x01 );
    option->add( "Monochrome (green)", 0x02 );
    option->add( "Monochrome (color overlay)", 0x05 );
    option->add( "Color (with PROM)", 0x04 );

    option = group->add( OptRedScreen, "Bomb effect (red screen)", 0, 0x08 );
    option->add( S_Enabled, 0x08 );
    option->add( S_Disabled, 0x00 );

    option = group->add( OptAnalogSounds, "Analog sound emulation", 0, 0x10 );
    option->add( S_Enabled, 0x10 );
    option->add( S_Disabled, 0x00 );

    // Setup the option handler
    optionHandler()->add( &main_board_->port2i_, ui, OptLives, OptBonus, OptCoinInfo );
    optionHandler()->add( &settings_, ui, OptScreenType, OptRedScreen, OptAnalogSounds );

    // Ask notification so internal variables are same as GUI
    return true;
}

SpaceInvaders::SpaceInvaders( SpaceInvadersBoard * board ) :
    walk_timer_( 0, 75000.0, 0.0000001 ),   // RA=variable, RB=75 Kohm, C=0.1 uF
    walk_rcfilter_1_( 100.0, 0.0000047 ),   // R=100 ohm, C=4.7 uF
    walk_rcfilter_2_( 100.0, 0.000010 ),    // R=100 ohm, C=10 uF
    extend_play_timer_( 100000.0, 47000.0, 0.000001 ), // RA=100 Kohm, RB=47 Kohm, C=1 uF
    ufo_hit_( 78, 85, 5.5 ),    // Data obtained by simulation in LTSpice
    target_hit_( 4, 188, 4.0 ), // Data obtained by simulation in LTSpice (a value of 3 for the first parameter may be more accurate)
    shot_( 8400 * 2 ),
    flash_( 8400 / 2 ),
    flash_rc1_( flash_, 560, Micro(0.1) ),
    flash_rc2_( flash_rc1_, Kilo(6.8), Micro(0.1) )
{
    int i;

    createScreen( ScreenWidth, ScreenHeight, ScreenColors );

    // First entry (zero) is black for all palettes
    for( i=0; i<8; i++ ) {
        palette_data_[i][0] = TPalette::encodeColor(0,0,0);
    }

    // Set other colors
    for( i=1; i<8; i++ ) {
        // Palette 0 is black/white
        palette_data_[0][i] = TPalette::encodeColor(255,255,255);

        // Palette 1 is black/light cyan
        palette_data_[1][i] = TPalette::encodeColor(0,169,219);

        // Palette 2 is black/green
        palette_data_[2][i] = TPalette::encodeColor(0,255,0);

        // Palette 3 is black/red (for "all screen red" effect)
        palette_data_[3][i] = TPalette::encodeColor(255,0,0);

        // Palette 4 is for color PROMs
        unsigned r = (i & 0x01) ? 0xFF : 0x00;
        unsigned g = (i & 0x04) ? 0xFF : 0x00;
        unsigned b = (i & 0x02) ? 0xFF : 0x00;
        palette_data_[4][i] = TPalette::encodeColor(r,g,b);

        // Palette 5 is for color overlays (desaturated colors) and is set later
        palette_data_[5][i] = 0;

        // Palette 6 is black/cyan
        palette_data_[6][i] = TPalette::encodeColor(0,255,255);

        // Palette 7 is unused
        palette_data_[7][i] = 0;
    }

    // Set overlay palette
    palette_data_[5][1] = TPalette::encodeColor( 222, 90, 101 );    // Red
    palette_data_[5][3] = TPalette::encodeColor( 255, 252, 146 );   // Yellow
    palette_data_[5][4] = TPalette::encodeColor( 37, 153, 116 );    // Green
    palette_data_[5][7] = TPalette::encodeColor( 240, 240, 240 );   // White

    // Initialize analog circuits
    walk_timer_.setAmplitude( 255, 0 );

    extend_play_timer_.setAmplitude( ~0, 0 ); // On/off (used in "AND" mode)
    extend_play_tone_.setFrequency( 480 ); // 480 Hz square wave
    extend_play_tone_.setAmplitude( 180, 0 );

    ufo_sound_.setSLF( 120000, 0.000001 ); // 120K, 1u
    ufo_sound_.setVCO( 3900, 0.00000022, 0, 0 ); // 3.9K, 0.22u (alternate: 8.2K, 0.1u)
    ufo_sound_.setVCO_Select( 1 ); // 0=external, 1=internal
    ufo_sound_.setEnvelopeControl( 100000, 0, 0 ); // 100K, unconnected, unconnected
    ufo_sound_.setAmplifier( 10000, 150000 ); // 10K, 150K
    ufo_sound_.setMixer( snMixer_VCO );
    
    shot_active_ = 0;
    
    flash_active_ = 0;

    // Initialize board
    main_board_ = board != 0 ? board : new SpaceInvadersBoard();
    main_board_->setVram( screen()->bits()->data() );
    active_palette_ = 0xFF;
    settings_ = 0;

    // Create a color PROM for games that do not have one
    memset( main_board_->color_prom_, 7, 0x400 );
    for( i=0; i<32; i++ ) memset( main_board_->color_prom_+i*32+ 2, 4, 7 );
    for( i=0; i<32; i++ ) memset( main_board_->color_prom_+i*32+24, 1, 4 );

    eventHandler()->add( idCoinSlot1,        ptNormal, &main_board_->port1i_, 0x01 );
    eventHandler()->add( idKeyStartPlayer2,  ptNormal, &main_board_->port1i_, 0x02 );
    eventHandler()->add( idKeyStartPlayer1,  ptNormal, &main_board_->port1i_, 0x04 );
    eventHandler()->add( idKeyP1Action1,     ptNormal, &main_board_->port1i_, 0x10 );
    eventHandler()->add( idKeyP2Action1,     ptNormal, &main_board_->port2i_, 0x10 );

    setJoystickHandler( 0, new TJoystickToPortHandler(idJoyP1Joystick1, ptNormal, jm4Way, &main_board_->port1i_, 0x00004020) );
    setJoystickHandler( 1, new TJoystickToPortHandler(idJoyP2Joystick1, ptNormal, jm4Way, &main_board_->port2i_, 0x00004020) );

    // Register this driver
    registerDriver( SpaceInvadersInfo );
}

void SpaceInvaders::setActivePalette( unsigned char index )
{
    if( active_palette_ != index ) {
        active_palette_ = index;
        for( int i=0; i<8; i++ ) {
            palette()->setColor( i, palette_data_[index][i] );
        }
    }
}

void SpaceInvaders::repaintScreen()
{
    // The following code force a refresh of the video RAM, which in
    // turn updates our screen bitmap
    for( unsigned addr=0x2400; addr<0x4000; addr++ ) {
        main_board_->writeByte( addr, main_board_->ram_[addr] );
    }
}

void SpaceInvaders::run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate )
{
    unsigned char old_port3o = main_board_->port3o_;
    unsigned char old_port5o = main_board_->port5o_;

    main_board_->run();
    
    // If the amplifier is enabled, play the sounds
    if( main_board_->port3o_ & 0x20 ) {
        if( settings_ & 0x10 ) {
            // Analog sound emulation
            if( (main_board_->port5o_ & 0x10) && ((old_port5o & 0x10) == 0) ) {
                ufo_hit_.play();
            }
            else if( ((main_board_->port5o_ & 0x10) == 0) && (old_port5o & 0x10) ) {
                ufo_hit_.fade( 0.500 );
            }
            
            if( (main_board_->port3o_ & 0x08) && ((old_port3o & 0x08) == 0) ) {
                target_hit_.play( 0.250, 0.050 ); 
            }
            
            if( (main_board_->port3o_ & 0x02) && ((old_port3o & 0x02) == 0) ) {
                shot_active_ = 15;
            }
            
            if( (main_board_->port3o_ & 0x04) && ((old_port3o & 0x04) == 0) ) {
                flash_active_ = 75;
                frame->playForceFeedbackEffect( 0, fxFF_Explosion, 0 );
            }

            renderAnalogSounds( frame, samplesPerFrame, samplingRate );
        }
        else {
            // Sample-based sound emulation
            if(  main_board_->port3o_ & 0x01 ) samples_[0].play(pmLooping); else samples_[0].stop();// Ufo ship
            if( (main_board_->port3o_ & 0x04) && ((old_port3o & 0x04) == 0) ) samples_[2].play();	// Base hit (flash)
            if( (main_board_->port3o_ & 0x10) && ((old_port3o & 0x10) == 0) ) samples_[9].play();	// Extended play
            if( (main_board_->port5o_ & 0x01) && ((old_port5o & 0x01) == 0) ) samples_[4].play();	// Walk 1
            if( (main_board_->port5o_ & 0x02) && ((old_port5o & 0x02) == 0) ) samples_[5].play();	// Walk 2
            if( (main_board_->port5o_ & 0x04) && ((old_port5o & 0x04) == 0) ) samples_[6].play();	// Walk 3
            if( (main_board_->port5o_ & 0x08) && ((old_port5o & 0x08) == 0) ) samples_[7].play();	// Walk 4
            if( (main_board_->port5o_ & 0x10) && ((old_port5o & 0x10) == 0) ) samples_[8].play();   // Ufo hit
            if( (main_board_->port3o_ & 0x08) && ((old_port3o & 0x08) == 0) ) samples_[3].play();   // Target hit
            if( (main_board_->port3o_ & 0x02) && ((old_port3o & 0x02) == 0) ) samples_[1].play();   // Shot
            
            for( int i=0; i<10; i++ ) {
                samples_[i].mix( frame->getMixer(), chMono, samplesPerFrame, samplingRate );
            }
        }
    }

    // Set palette and render the video
    if( (settings_ & 0x08) && (main_board_->port3o_ & 0x04) ) {
        setActivePalette( 3 ); // Red screen
    }
    else {
        setActivePalette( settings_ & 0x07 ); // User settings
    }

    frame->setVideo( screen() );
}

// Render the game sound by circuit emulation
void SpaceInvaders::renderAnalogSounds( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate )
{
    int voices = 0;
    TMixerBuffer * mixer_buffer = frame->getMixer()->getBuffer( chMono, samplesPerFrame, 0 );

    // Ufo ship
    if( main_board_->port3o_ & 0x01 ) {
        ufo_sound_.enableOutput( true );
        ufo_sound_.playSound( mixer_buffer->data(), samplesPerFrame, samplingRate );
        voices++;
    }
    else {
        ufo_sound_.enableOutput( false );
    }

    // Walk sound circuit
    if( main_board_->port5o_ & 0x0F ) {
        if( ! walk_timer_.active() ) {
            double wr = 0;

            // Load resistors according to port settings
            if( main_board_->port5o_ & 0x01 ) wr +=  1.0 /  40;
            if( main_board_->port5o_ & 0x02 ) wr +=  1.0 /  68;
            if( main_board_->port5o_ & 0x04 ) wr +=  1.0 /  82;
            if( main_board_->port5o_ & 0x08 ) wr +=  1.0 / 100;
            
            const double DiodeFactor = 1.4;

            walk_timer_.setVariableResistor( Kilo((1/wr)*DiodeFactor) ); // Quick and dirty: account for diodes too
            walk_timer_.setActive( true );
        }

        // Play the sound in a separate buffer
        walk_sound_buffer_.expand( samplesPerFrame );
        walk_timer_.setBuffer( walk_sound_buffer_.data(), samplesPerFrame, samplingRate );
        walk_rcfilter_1_.apply( walk_sound_buffer_.data(), samplesPerFrame, samplingRate );
        walk_rcfilter_2_.apply( walk_sound_buffer_.data(), samplesPerFrame, samplingRate );

        // Mix into the primary buffer
        TMixer::mix( mixer_buffer->data(), walk_sound_buffer_.data(), samplesPerFrame );
        voices++;
    }
    else {
        if( walk_timer_.active() ) {
            // Let the sound continue for a while (to somewhat compensate for the fact
            // that we are undersampling at 60 Hz while the game runs at 120 Hz), but
            // always run the filters for the whole frame because of the "buffer" effect 
            // introduced by capacitors
            walk_sound_buffer_.expand( samplesPerFrame );
            walk_sound_buffer_.clear();
            walk_timer_.setBuffer( walk_sound_buffer_.data(), samplesPerFrame * 3 / 4, samplingRate );
            walk_rcfilter_1_.apply( walk_sound_buffer_.data(), samplesPerFrame, samplingRate );
            walk_rcfilter_2_.apply( walk_sound_buffer_.data(), samplesPerFrame, samplingRate );

            // Mix into the primary buffer
            TMixer::mix( mixer_buffer->data(), walk_sound_buffer_.data(), samplesPerFrame );
            voices++;

            // Stop the circuit
            walk_timer_.setActive( false );
            walk_rcfilter_1_.reset();
            walk_rcfilter_2_.reset();
        }
    }

    // Extended play sound circuit
    if(main_board_->port3o_ & 0x10 ) {
        if( ! extend_play_timer_.active() ) {
            extend_play_timer_.setActive( true );
            extend_play_vibrato_offset_ = 0;
        }

        // Play the tone and combine it with the timer
        extend_play_sound_buffer_.expand( samplesPerFrame );
        extend_play_tone_.setBuffer( extend_play_sound_buffer_.data(), samplesPerFrame, samplingRate );
        extend_play_timer_.andWithBuffer( extend_play_sound_buffer_.data(), samplesPerFrame, samplingRate );

        int * data = extend_play_sound_buffer_.data();

        // Add the vibrato effect if enabled and adjust volume
        for( unsigned n=0; n<samplesPerFrame; n++ ) {
#ifdef EXTEND_PLAY_VIBRATO        
            double extend_play_wave_period = 2 * 3.1415926535 / (samplingRate / 60); // 60 Hz
            int va = (sin(extend_play_vibrato_offset_*extend_play_wave_period) * ExtendPlayVibratoAmplitude);
            if( *data > 0 ) *data += va; else *data -= va;
#endif
            
            *data = *data / 4; // Set proper volume

            data++;
            extend_play_vibrato_offset_++;
        }

        // Mix it all into the primary buffer
        TMixer::mix( mixer_buffer->data(), extend_play_sound_buffer_.data(), samplesPerFrame );
        voices++;
    }
    else {
        extend_play_timer_.setActive( false );
        extend_play_tone_.reset();
    }

    // Flash (base hit) circuit
    if( flash_active_ ) {
        flash_active_--;
        if( flash_active_ == 0 ) frame->stopForceFeedbackEffect( 0 );
        
        flash_rc2_.resetStream();
        flash_rc2_.updateTo( samplesPerFrame );
        flash_rc2_.mixStream( mixer_buffer->data(), 150, 0, 200 );
        voices++;
    }
    
    // Ufo hit and target hit
    if( ! ufo_hit_.stopped() ) {
        ufo_hit_.mix( mixer_buffer->data(), samplesPerFrame );
        voices++;
    }
    
    if( ! target_hit_.stopped() ) {
        target_hit_.mix( mixer_buffer->data(), samplesPerFrame );
        voices++;
    }
    
    // Shot
    if( shot_active_ ) {
        shot_active_--;
        shot_.resetStream();
        shot_.updateTo( samplesPerFrame );
        shot_.mixStream( mixer_buffer->data(), 150, 0, 15 );
        voices++;
    }

    mixer_buffer->setVoices( TMath::max(2,voices) );
}

void SpaceInvaders::reset()
{
    main_board_->reset();
}

SpaceInvadersBoard::SpaceInvadersBoard()
{
    vram_ = 0;
    cpu_ = new I8080( *this );

    memset( ram_+0x0000, 0, 0x2000 );  // Clear the ROM area
    memset( ram_+0x4000, 0, 0x2000 );  // Clear the ROM area

    port0i_ = 0x40;
    port1i_ = 0;
    port2i_ = 0; // DIP switches and player 2 controls
    port2o_ = 0;
    port3o_ = 0;
    port4lo_ = 0;
    port4hi_ = 0;
    port5o_ = 0;

    reset();
}

SpaceInvadersBoard::~SpaceInvadersBoard()
{
    delete cpu_;
}

void SpaceInvadersBoard::run()
{
    cpu_->setCycles( 0 );

    // Before a frame is fully rendered, two interrupts have to occur
    for( int i=0; i<2; i++ ) {
        cpu_->run( CpuCyclesPerInterrupt );

        // Call the proper interrupt (0xD7 is RST 2 and jumps to 0x10,
        // 0xCF is RST 1 and jumps to 0x08)
        cpu_->interrupt( i ? 0xD7 : 0xCF );
    }
}

void SpaceInvadersBoard::reset()
{
    cpu_->reset();
}

unsigned char SpaceInvadersBoard::readByte( unsigned addr )
{
    addr &= 0xFFFF;
    
    return (addr < sizeof(ram_)) ? ram_[addr] : 0;
}

void SpaceInvadersBoard::writeByte( unsigned addr, unsigned char b )
{
    addr &= 0xFFFF;

    if( (addr >= 0x2000) && addr < 0x4000 ) {
        ram_[addr] = b;

        if( addr >= 0x2400 ) {
            // This is a write to video memory. Since the video screen is rotated, 
            // consecutive bits correspond to vertically consecutive pixels.
            // This is accounted for in the following code, and an extra buffer
            // is used to store the video memory in a more useable form.
		    addr -= 0x2400;

            unsigned x = addr / 32;
            unsigned y = addr % 32;
            unsigned char * vram = vram_ + ((255-y*8)*224) + x;
            unsigned char color = color_prom_[ 0x80 + y + ((x / 8) << 5)] & 0x07;
            for( int i=0; i<8; i++ ) {
                *vram = b & 1 ? color : 0;
			    vram -= ScreenWidth;
                b >>= 1;
            }
        }
    }
}

unsigned char SpaceInvadersBoard::readPort( unsigned port )
{
    unsigned char result = 0;

    switch( port ) {
    case 0:
        result = port0i_;
        break;
    case 1: 
        result = port1i_;
        break;
    case 2: 
        result = port2i_;
        break;
    case 3: 
        result = (unsigned char)(((((unsigned)port4hi_ << 8) | port4lo_) << port2o_) >> 8);
        break;
    }

    return result;
}

void SpaceInvadersBoard::writePort( unsigned addr, unsigned char b )
{
    switch( addr ) {
    case 2:
        port2o_ = b;
        break;
    case 3:
        port3o_ = b;
        break;
    case 4:
        port4lo_ = port4hi_;
        port4hi_ = b;
        break;
    case 5:
        port5o_ = b;
        break;
    case 6:
        // Watchdog timer
        break;
    }
}

bool SpaceInvadersPartII::initialize( TMachineDriverInfo * info )
{
    SpaceInvaders::initialize( info );

    // Update resources
    resourceHandler()->replace( Ef1, "pv.01", 0x00000000 );
    resourceHandler()->replace( Ef2, "pv.02", 0x00000000 );
    resourceHandler()->replace( Ef3, "pv.03", 0x00000000 );
    resourceHandler()->replace( Ef4, "pv.04", 0x00000000 );
    resourceHandler()->add( Ef5, "pv.05", 0x800, efROM, main_board_->ram_+0x4000 );
    resourceHandler()->add( EfPROM1, "pv06_1.bin", 0x0400, efColorPROM, main_board_->color_prom_+0x0000 );
    resourceHandler()->add( EfPROM2, "pv07_2.bin", 0x0400, efColorPROM, main_board_->color_prom_+0x0400 );

    resourceHandler()->addToMachineDriverInfo( info );

    // Update user interface
    TUserInterface * ui = info->userInterface();

    TUiOption * option = ui->replaceOption( OptLives, "Ships per game", 0, 0x03 );
    option->add( "3", 0x00 );
    option->add( "4", 0x01 );
    option->add( S_Unused, 0x02 );
    option->add( S_Unused, 0x03 );

    option = ui->replaceOption( OptBonus, "High score preset mode", 0 , 0x08 );
    option->add( S_Off, 0x00 );
    option->add( S_On, 0x08 );

    ui->removeOption( OptCoinInfo );

    optionHandler()->resynch(ui);

    return true;
}

SpaceInvadersPartII::SpaceInvadersPartII() : SpaceInvaders() 
{
    registerDriver( SpaceInvadersPartIIInfo );
}

bool SpaceInvadersDeluxe::initialize( TMachineDriverInfo * info )
{
    SpaceInvadersPartII::initialize( info );

    // Update resources
    resourceHandler()->replace( Ef1, "invdelux.h", 0x00000000 );
    resourceHandler()->replace( Ef2, "invdelux.g", 0x00000000 );
    resourceHandler()->replace( Ef3, "invdelux.f", 0x00000000 );
    resourceHandler()->replace( Ef4, "invdelux.e", 0x00000000 );
    resourceHandler()->replace( Ef5, "invdelux.d", 0x00000000 );
    
    resourceHandler()->addToMachineDriverInfo( info );

    return true;
}

SpaceInvadersDeluxe::SpaceInvadersDeluxe() : SpaceInvadersPartII() 
{
    registerDriver( SpaceInvadersDeluxeInfo );
}

bool LunarRescue::initialize( TMachineDriverInfo * info )
{
    SpaceInvaders::initialize( info );

    // Update resources
    resourceHandler()->replace( Ef1, "lrescue.1", 0x00000000 );
    resourceHandler()->replace( Ef2, "lrescue.2", 0x00000000 );
    resourceHandler()->replace( Ef3, "lrescue.3", 0x00000000 );
    resourceHandler()->replace( Ef4, "lrescue.4", 0x00000000 );
    resourceHandler()->add( Ef5, "lrescue.5", 0x800, efROM, main_board_->ram_+0x4000 );
    resourceHandler()->add( Ef6, "lrescue.6", 0x800, efROM, main_board_->ram_+0x4800 );
    resourceHandler()->add( EfPROM1, "7643-1.cpu", 0x0400, efColorPROM, main_board_->color_prom_+0x0000 );

    resourceHandler()->addToMachineDriverInfo( info );

    // Update user interface
    TUserInterface * ui = info->userInterface();

    ui->removeOption( OptBonus );
    ui->removeOption( OptCoinInfo );

    return true;
}

LunarRescue::LunarRescue() : SpaceInvaders() 
{
    registerDriver( LunarRescueInfo );
}

bool SpaceAttackII::initialize( TMachineDriverInfo * info )
{
    SpaceInvaders::initialize( info );

    // Update resources
    resourceHandler()->replace( Ef1, "spaceatt.h", 0x00000000 );
    resourceHandler()->replace( Ef2, "spaceatt.g", 0x00000000 );
    resourceHandler()->replace( Ef3, "spaceatt.f", 0x00000000 );
    resourceHandler()->replace( Ef4, "spaceatt.e", 0x00000000 );

    resourceHandler()->addToMachineDriverInfo( info );

    // Update user interface
    TUserInterface * ui = info->userInterface();

    TUiOption * option = ui->replaceOption( OptCoinInfo, "Coin", 0, 0x80 );
    option->add( S_1Coin1Play, 0x00 );
    option->add( S_2Coin1Play, 0x80 );

    return true;
}

SpaceAttackII::SpaceAttackII() : SpaceInvaders() 
{
    registerDriver( SpaceAttackIIInfo );
}

bool OzmaWars::initialize( TMachineDriverInfo * info )
{
    SpaceInvaders::initialize( info );

    // Update resources
    resourceHandler()->replace( Ef1, "mw01", 0x00000000 );
    resourceHandler()->replace( Ef2, "mw02", 0x00000000 );
    resourceHandler()->replace( Ef3, "mw03", 0x00000000 );
    resourceHandler()->replace( Ef4, "mw04", 0x00000000 );
    resourceHandler()->add( Ef5, "mw05", 0x800, efROM, main_board_->ram_+0x4000 );
    resourceHandler()->add( Ef6, "mw06", 0x800, efROM, main_board_->ram_+0x4800 );

    resourceHandler()->addToMachineDriverInfo( info );

    // Update user interface
    TUserInterface * ui = info->userInterface();

    TUiOption * option = ui->replaceOption( OptLives, "Energy", 0, 0x03 );
    option->add( "15000", 0x00 );
    option->add( "20000", 0x01 );
    option->add( "25000", 0x02 );
    option->add( "35000", 0x03 );

    option = ui->replaceOption( OptBonus, "Bonus energy", 0, 0x08 );
    option->add( "15000", 0x00 );
    option->add( "10000", 0x08 );

    option = ui->replaceOption( OptCoinInfo, "Coin", 0, 0x80 );
    option->add( S_1Coin1Play, 0x00 );
    option->add( S_2Coin1Play, 0x80 );

    // Ask notification so internal variables are same as GUI
    return true;
}

OzmaWars::OzmaWars() : SpaceInvaders() 
{
    registerDriver( OzmaWarsInfo );
}

RollingCrashBoard::RollingCrashBoard() :
    SpaceInvadersBoard()
{
    memset( color_ram_, 0, sizeof(color_ram_) );
    memset( extra_ram_, 0, sizeof(extra_ram_) );
    port0o_ = 0;
}

unsigned char RollingCrashBoard::readByte( unsigned addr )
{
    if( addr >= 0xA000 && addr <= 0xBFFF ) {
        return color_ram_[ remapColorRamAddr(addr-0xA000) ];
    }
    else if( addr >= 0xE400 && addr <= 0xFFFF ) {
        return extra_ram_[ addr-0xE400 ];
    }
    
    return SpaceInvadersBoard::readByte(addr);
}

void RollingCrashBoard::writeByte( unsigned addr, unsigned char b )
{
    if( addr >= 0xA000 && addr <= 0xBFFF ) {
        color_ram_[ remapColorRamAddr(addr-0xA000) ] = b;
    }
    else if( addr >= 0xE400 && addr <= 0xFFFF ) { 
        extra_ram_[ addr-0xE400 ] = b;
    }
    else {
        SpaceInvadersBoard::writeByte(addr, b);
    }
}

unsigned char RollingCrashBoard::readPort( unsigned port )
{
    if( port == 0 ) {
        // Copy the "RIGHT" and "LEFT" inputs to this port, otherwise it is not possible to select
        // between Rolling Crash and Moon Base at startup
        return 0xFF - ((port1i_ & 0x60) >> 4);
    }
    
    return SpaceInvadersBoard::readPort(port);
}

void RollingCrashBoard::writePort( unsigned port, unsigned char b )
{
    if( port == 0 ) {
        // Sounds for Rolling Crash
        // 0x02 = our car is changing lane
        // 0x04 = crash
        // 0x10 = enemy car is chasing
        port0o_ = b;
        port0o_written_ = true;
    }
    else if( port == 1 ) {
        port1o_written_ = true;
    }
    
    SpaceInvadersBoard::writePort(port, b);
}

void RollingCrashBoard::run()
{
    port0o_written_ = false;
    port1o_written_ = false;

    SpaceInvadersBoard::run();
    
    // Note to self: if either port 0 or port 1 has been written then the current game is Rolling Crash, otherwise it's Moon Base
    
    // Remap sounds to standard port (port 0 is used only by Rolling Crash)
    if( port0o_written_ ) {
        port3o_ = (port3o_ & ~0x07) | (port0o_ & 0x06) | ((port0o_ >> 4) & 1);
    }

    // The flyer for Rolling Crash shows that red and green are swapped
    static unsigned MapGBR[8] = {0,4,2,6,1,3,5,7};
    
    // Draw the entire screen
    for( unsigned addr=0x0000; addr<0x1C00; addr++ ) {
        unsigned x = addr / 32;
        unsigned y = addr % 32;
        unsigned offset = remapColorRamAddr(addr+0x400);
        unsigned char color = color_ram_[offset] & 0x07;
        color = MapGBR[color];
        unsigned char * vram = vram_ + ((255-y*8)*224) + x;
        unsigned char b = ram_[addr+0x2400];
        for( int i=0; i<8; i++ ) {
            *vram = b & 1 ? color : 0;
            vram -= ScreenWidth;
            b >>= 1;
        }
    }
}

bool RollingCrash::initialize( TMachineDriverInfo * info )
{
    SpaceInvaders::initialize( info );
    
    // Update resources
    resourceHandler()->clear();

    resourceHandler()->add( Ef1,    "rc01.bin", 0x400, efROM, main_board_->ram_ );
    resourceHandler()->add( Ef1+99, "rc02.bin", 0x400, efROM, main_board_->ram_ + 0x0400 );
    resourceHandler()->add( Ef2,    "rc03.bin", 0x400, efROM, main_board_->ram_ + 0x0800 );
    resourceHandler()->add( Ef2+99, "rc04.bin", 0x400, efROM, main_board_->ram_ + 0x0C00 );
    resourceHandler()->add( Ef3,    "rc05.bin", 0x400, efROM, main_board_->ram_ + 0x1000 );
    resourceHandler()->add( Ef3+99, "rc06.bin", 0x400, efROM, main_board_->ram_ + 0x1400 );
    resourceHandler()->add( Ef4,    "rc07.bin", 0x400, efROM, main_board_->ram_ + 0x1800 );
    resourceHandler()->add( Ef4+99, "rc08.bin", 0x400, efROM, main_board_->ram_ + 0x1C00 );
    
    resourceHandler()->add( Ef5,    "rc09.bin", 0x800, efROM, main_board_->ram_+0x4000 );
    resourceHandler()->add( Ef5+99, "rc10.bin", 0x800, efROM, main_board_->ram_+0x4800 );
    resourceHandler()->add( Ef6,    "rc11.bin", 0x800, efROM, main_board_->ram_+0x5000 );
    resourceHandler()->add( Ef6+99, "rc12.bin", 0x800, efROM, main_board_->ram_+0x5800 );
    
    resourceHandler()->addToMachineDriverInfo( info );
    
    // Update user interface
    TUserInterface * ui = info->userInterface();
    
    ui->removeOption( OptBonus ); // According to flyer, bonus is at 5000 points for Rolling Crash and 2000 points for Moon Base
    ui->removeOption( OptCoinInfo );
    
    // Ask notification so internal variables are same as GUI
    return true;
}

// The ROM actually contains two games, press "RIGHT" or "LEFT" at startup to select which one to run
RollingCrash::RollingCrash() : SpaceInvaders( new RollingCrashBoard ) 
{
    registerDriver( RollingCrashInfo );
}
