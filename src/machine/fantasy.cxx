/*
    Fantasy arcade machine emulator

    Copyright (c) 2011 Alessandro Scotti
*/
#include "fantasy.h"

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
    Smp_0 = 0, Smp_1, Smp_2, Smp_3, Smp_4, Smp_5, Smp_6, Smp_7, Smp_8, Smp_9, Smp_A, Smp_B,
    Smp_ShotA, Smp_Bomb,
    Sk4_07, Sk4_08, Sk4_09, Sk4_10, Sk4_12, Sk4_14, Sk4_15, Sk4_16, Sk4_17,
    Sk5_7, Sk5_6,
    Sk5_50, Sk5_51,
    Snd_51, Snd_52, Snd_53,
    Sk6_07, Sk6_08, Sk6_11,
    OptLives, OptContinue, OptCoinage
};

// Machine info
static TMachineInfo FantasyInfo = { 
    "fantasy", "Fantasy", S_SNK, 1981, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TMachineInfo FantasyUsInfo = { 
    "fantasyu", "Fantasy (US)", S_RockOla, 1981, ScreenWidth, ScreenHeight, ScreenColors, VideoFrequency
};

static TGameRegistrationHandler reg( &FantasyUsInfo, Fantasy::createInstance );

bool Fantasy::initialize( TMachineDriverInfo * info )
{
    resourceHandler()->clear();

#ifdef FANTASY
    resourceHandler()->add( Sk4_12, "5.12",     0x1000, efROM, main_board_->ram_+0x3000 );
    resourceHandler()->add( Sk4_07, "1.7",      0x1000, efROM, main_board_->ram_+0x4000 );
    resourceHandler()->add( Sk4_08, "2.8",      0x1000, efROM, main_board_->ram_+0x5000 );
    resourceHandler()->add( Sk4_09, "3.9",      0x1000, efROM, main_board_->ram_+0x6000 );
    resourceHandler()->add( Sk4_10, "4.10",     0x1000, efROM, main_board_->ram_+0x7000 );
    resourceHandler()->add( Sk4_14, "ic14.cpu", 0x1000, efROM, main_board_->ram_+0x8000 );
    resourceHandler()->add( Sk4_15, "ic15.cpu", 0x1000, efROM, main_board_->ram_+0x9000 );
    resourceHandler()->add( Sk4_16, "8.16",     0x1000, efROM, main_board_->ram_+0xA000 );
    resourceHandler()->add( Sk4_17, "9.17",     0x1000, efROM, main_board_->ram_+0xB000 );
#else // FANTASYU
    resourceHandler()->add( Sk4_12, "ic12.cpu", 0x1000, efROM, main_board_->ram_+0x3000 );
    resourceHandler()->add( Sk4_07, "ic07.cpu", 0x1000, efROM, main_board_->ram_+0x4000 );
    resourceHandler()->add( Sk4_08, "ic08.cpu", 0x1000, efROM, main_board_->ram_+0x5000 );
    resourceHandler()->add( Sk4_09, "ic09.cpu", 0x1000, efROM, main_board_->ram_+0x6000 );
    resourceHandler()->add( Sk4_10, "ic10.cpu", 0x1000, efROM, main_board_->ram_+0x7000 );
    resourceHandler()->add( Sk4_14, "ic14.cpu", 0x1000, efROM, main_board_->ram_+0x8000 );
    resourceHandler()->add( Sk4_15, "ic15.cpu", 0x1000, efROM, main_board_->ram_+0x9000 );
    resourceHandler()->add( Sk4_16, "ic16.cpu", 0x1000, efROM, main_board_->ram_+0xA000 );
    resourceHandler()->add( Sk4_17, "ic17.cpu", 0x1000, efROM, main_board_->ram_+0xB000 );
#endif
    
    resourceHandler()->add( Sk5_7, "fantasy.ic7",  0x20, efPalettePROM, palette_prom_ );
    resourceHandler()->add( Sk5_6, "fantasy.ic6",  0x20, efPalettePROM, palette_prom_+0x20 );
    
    resourceHandler()->add( Sk5_50, "fs10ic50.bin", 0x1000, efVideoROM, main_board_->ram_+0xC000 );
    resourceHandler()->add( Sk5_51, "fs11ic51.bin", 0x1000, efVideoROM, main_board_->ram_+0xD000 );
    
    resourceHandler()->add( Snd_51, "fs_b_51.bin", 0x800, efROM, main_board_->sound_rom_ );
    resourceHandler()->add( Snd_52, "fs_a_52.bin", 0x800, efROM, main_board_->sound_rom_+0x0800 );
    resourceHandler()->add( Snd_53, "fs_c_53.bin", 0x800, efROM, main_board_->sound_rom_+0x1000 );
    
    resourceHandler()->add( Smp_0, "ft_voi-0.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_1, "ft_voi-1.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_2, "ft_voi-2.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_3, "ft_voi-3.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_4, "ft_voi-4.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_5, "ft_voi-5.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_6, "ft_voi-6.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_7, "ft_voi-7.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_8, "ft_voi-8.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_9, "ft_voi-9.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_A, "ft_voi-a.wav", 0x0000, efSample, 0 );
    resourceHandler()->add( Smp_B, "ft_voi-b.wav", 0x0000, efSample, 0 );

    resourceHandler()->add( Sk6_07, "fs_d_7.bin",  0x800, efSoundPROM, speech_rom_ );
    resourceHandler()->add( Sk6_08, "fs_e_8.bin",  0x800, efSoundPROM, speech_rom_+0x0800 );
    resourceHandler()->add( Sk6_11, "fs_f_11.bin", 0x800, efSoundPROM, speech_rom_+0x1000 );
    
    resourceHandler()->addToMachineDriverInfo( info );
    
    // Setup user interface
    TUserInterface * ui = info->userInterface();
    
    TUiOptionGroup * group = ui->addGroup( S_DSW_1, dtDipSwitch );
    
    TUiOption * option = group->add( OptLives, S_Lives, 0, 0x30 );
    option->add( "3", 0x00 );
    option->add( "4", 0x10 );
    option->add( "5", 0x20 );
    
    option = group->add( OptCoinage, S_Coinage, 0, 0x40 );
    option->add( S_1Coin1Play, 0x00 );
    option->add( S_2Coin1Play, 0x40 );
    
    option = group->add( OptContinue, "Allow continue", 0, 0x80 );
    option->add( S_On,  0x00 );
    option->add( S_Off, 0x80 );
    
    optionHandler()->add( &main_board_->dsw0_, ui, OptLives, OptCoinage );
    
    // Ask notification so internal variables are same as GUI
    return true;
}

static const unsigned SpeechAddressTable[12] = {
    0x04000, // How are you? I'm fine, thank you!
    0x04297, // Jane? I feel something abnormal
    0x044b6, // Help! Help! Help!
    0x04682, // Uaargh!
    0x04927, // 'oom, wala wala wala!
    0x04be0, // I love you!
    0x04cc2, // I love you too!
    0x04e36, // Let's go
    0x05000, // Aaaaaaahh!
    0x05163, // Ha ha ha ha ha ha ha!
    0x052c9, // Help me!
    0x053fd
};

Fantasy::Fantasy( FantasyBoard * board ) :
    Nibbler( board )
{
    registerDriver( FantasyInfo );
    registerDriver( FantasyUsInfo );
    
    board->hd38880_.setSamples( 12, SpeechAddressTable, board->speech_samples_ );
}

bool Fantasy::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    if( id >= Smp_0 && id <= Smp_B ) {
        ((FantasyBoard *)main_board_)->speech_samples_[ id-Smp_0 ].setSample( copySample(buf,len) );
        return true;
    }

    return Nibbler::setResourceFile( id, buf, len );
}

void Fantasy::run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate )
{
    Nibbler::run( frame, samplesPerFrame, samplingRate );
    
    ((FantasyBoard *)main_board_)->hd38880_.play( frame, samplesPerFrame, samplingRate );
}

void FantasyBoard::writeByte( unsigned addr, unsigned char b )
{
    if( addr == 0x2400 ) {
        hd38880_.write( b );
    }
    else {
        NibblerBoard::writeByte( addr, b );
    }
}
