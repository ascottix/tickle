/*
    Vanguard arcade machine soundboard emulator

    Copyright (c) 2011 Alessandro Scotti
*/
#include <ase/ase_timer555_astable.h>
#include <emu/emu_math.h>

#include "vanguard_soundboard.h"

const int VanguardSoundBoardXtalClock = 11289000; // Reference clock is 11.289 MHz

const int VanguardSoundBoardTuneClock = ATimer555Astable::getFrequencyInHertz(Kilo(1), Kilo(18), Micro(1)); // 39 Hz

const int WaveformVoiceVolume = 15; // Max is 51

VanguardMusicChannel::VanguardMusicChannel( unsigned char * a_rom )
{
    rom = a_rom;
    
    reset();
}

void VanguardMusicChannel::reset()
{
    setMuted( true );
    setOneShot( false );
    setOffset( 0, 0xFF );
    setWaveform( 1 );
    waveform_counter = 0;
    waveform_step = 0;
}

void VanguardMusicChannel::setMuted( bool m )
{
    muted = m;
    if( m ) offset_curr = 0;
}

void VanguardMusicChannel::setOneShot( bool o )
{
    one_shot = o;
}

void VanguardMusicChannel::setOffset( unsigned o, unsigned m )
{
    offset = o;
    offset_mask = m;
}

void VanguardMusicChannel::nextOffset()
{
    offset_curr = (offset_curr + 1) & offset_mask;
    
    if( offset_curr == 0 && one_shot ) {
        muted = true;
    }
}

void VanguardMusicChannel::setPitchFromCurrentRomOffset( unsigned sampling_rate )
{
    if( ! muted ) {
        waveform_step = 0;
        unsigned char b = rom[offset + offset_curr];
        if( b != 0xFF ) {
            int clock = VanguardSoundBoardXtalClock / 32;
            waveform_step = (clock << 10) / sampling_rate;
            waveform_step /= (256-b);
        }
    }
}

void VanguardMusicChannel::setWaveform( int mask )
{
    /*
     The output of the 393 counter is connected to the following resistor network:
     
              QD       QC       QB       QA
              |        |        |        |
     +--------+        |        |        |
     |        |        |        |        |
    10K      10K      10K      10K      10K
     |        |        |        |        |
     |        |        |        |        |
     |        +--5.1K--+--5.1K--+--5.1K--+---10K--- GROUND
     |        |        |        |        
     |        |        |        |        
    SW1      SW2      SW3      SW4   
     |        |        |        |        
     |        |        |        |        
     +--------+--------+--------+-------> OUTPUT
     
     A 4-bit counter is connected to inputs QD (bit 3) to QA (bit 0) and
     the switches SW1 to SW4 allow the program to control what is sent to the output.
    */
    static double WaveformBySwitch[16*16] = {
        0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, // All switches off
        0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, // Bit 0: square
        0.00, 0.31, 0.62, 0.93, 1.25, 1.56, 1.87, 2.18, 2.52, 2.82, 3.14, 3.44, 3.77, 4.07, 4.39, 4.69, // Bit 1: sawtooth
        0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, // N/A
        0.00, 0.46, 0.94, 1.40, 1.89, 2.35, 2.82, 3.29, 1.25, 1.71, 2.19, 2.65, 3.14, 3.60, 4.07, 4.54, // Bit 2: double sawtooth
        0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, // N/A
        0.00, 0.41, 0.83, 1.24, 1.67, 2.08, 2.50, 2.92, 1.67, 2.08, 2.50, 2.92, 3.35, 3.76, 4.18, 4.59, // 1+2: double sawtooth (Nibbler)
        0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, // N/A
        0.00, 0.86, 1.73, 2.59, 0.94, 1.79, 2.67, 3.52, 0.62, 1.48, 2.35, 3.21, 1.56, 2.41, 3.29, 4.14, // Bit 3: quadruple sawtooth
        0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, // N/A
        0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, // N/A
        0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, // N/A
        0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, // N/A
        0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, // N/A
        0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, 5.00, // N/A
        0.00, 0.50, 1.00, 1.50, 1.00, 1.50, 2.00, 2.50, 2.00, 2.50, 3.00, 3.50, 3.00, 3.50, 4.00, 4.50  // 1+2+4+8: sawtooth (Fantasy)
    };
    
    int offset = 16 * (mask & 0x0F);
    
    for( int i=0; i<16; i++ ) {
        double v = WaveformBySwitch[ offset + i ];
        
        waveform[i] = (int) (v * WaveformVoiceVolume); // Multiplier is to set the proper voice volume
    }
}

void VanguardMusicChannel::setWaveformData( const int * data, int scale )
{
    for( int i=0; i<16; i++ ) {
        waveform[i] = data[i] * scale;
    }
}

VanguardSoundBoard::VanguardSoundBoard( unsigned char * rom, unsigned num_channels )
{
    unsigned max_channels = sizeof(ch_) / sizeof(ch_[0]);

    ch_count_ = TMath::min( num_channels, max_channels );
    
    for( unsigned i=0; i<max_channels; i++ ) {
        ch_[i] = i < ch_count_ ? new VanguardMusicChannel( rom + i*0x800 ) : 0;
    }
    
    tone_period_ = 0;
    tone_counter_ = 0;
    tone_step_ = 0;
}

VanguardSoundBoard::~VanguardSoundBoard()
{
    for( unsigned i=0; i<ch_count_; i++ ) {
        delete ch_[i];
    }
}

void VanguardSoundBoard::play( TMixerBuffer * mixer_buffer, unsigned samplesPerFrame, unsigned samplingRate )
{
    int * data_buffer = mixer_buffer->data();

    tone_period_ = (samplingRate << 10) / VanguardSoundBoardTuneClock; // Tone clock is set at 39 Hz
    tone_step_ = 1 << 10;
    
    while( samplesPerFrame > 0 ) {
        samplesPerFrame--;
        
        for( unsigned i=0; i<ch_count_; i++ ) {
            if( ! ch_[i]->muted ) {
                *data_buffer += ch_[i]->getNextWaveformSample();
            }
        }
        
        data_buffer++;
        
        tone_counter_ += tone_step_;
        
        if( tone_counter_ >= tone_period_ ) {
            tone_counter_ -= tone_period_;
            
            for( unsigned i=0; i<ch_count_; i++ ) {
                ch_[i]->nextOffset();
                ch_[i]->setPitchFromCurrentRomOffset(samplingRate);
            }
        }
    }

    mixer_buffer->addVoices( ch_count_ );
}

Hd38880_SimWithSamples::Hd38880_SimWithSamples()
{
    speech_table_len_ = 0;
    speech_table_ = 0;
    speech_samples_ = 0;
    
    reset();
}

void Hd38880_SimWithSamples::setSamples( int len, const unsigned * table, TSamplePlayer * samples )
{
    speech_table_len_ = len;
    speech_table_ = table;
    speech_samples_ = samples;
}

void Hd38880_SimWithSamples::reset()
{
    speech_command_ = 0;
    speech_data_ = 0;
    speech_data_len_ = 0;
    speech_data_ofs_ = 0;
    speech_address_ = (unsigned) -1;
}

enum {
    SpCommandADSET  = 2,
    SpCommandSTART  = 12,
    SpCommandSTOP   = 10,
    SpCommandINT1   = 4,
    SpCommandINT2   = 6,
    SpCommandSYSPD  = 8
};

void Hd38880_SimWithSamples::write( unsigned char b )
{
    if( (b & 0x30) == 0x30 ) {
        b &= 0x0F;
        
        if( speech_data_len_ > 0 ) {
            // Read data
            speech_data_ |= ((unsigned) b) << speech_data_ofs_;
            speech_data_ofs_ += 4;
            speech_data_len_--;
            
            if( speech_data_len_ == 0 && speech_command_ == SpCommandADSET ) {
                speech_address_ = speech_data_;
            }
        }
        else {
            // Execute command
            speech_command_ = b;
            speech_data_ = 0;
            speech_data_ofs_ = 0;
            
            switch( speech_command_ ) {
                case SpCommandADSET:
                    speech_data_len_ = 5;
                    break;
                case SpCommandSTART:
                    if( speech_samples_ ) for( int i=0; i<speech_table_len_; i++ ) {
                        speech_samples_[i].stop();
                        if( speech_table_[i] == speech_address_ ) {
                            speech_samples_[i].play();
                        }
                    }
                    break;
                case SpCommandSTOP:
                    for( int i=0; i<speech_table_len_; i++ ) {
                        speech_samples_[i].stop();
                    }
                    break;
                case SpCommandINT1:
                case SpCommandINT2:
                case SpCommandSYSPD:
                    speech_data_len_ = 1;
                    break;
            }
        }
    }
}


void Hd38880_SimWithSamples::play( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate )
{
    if( speech_samples_ ) for( int i=0; i<speech_table_len_; i++ ) {
        speech_samples_[i].mix( frame->getMixer(), chMono, samplesPerFrame, samplingRate );
    }
}

