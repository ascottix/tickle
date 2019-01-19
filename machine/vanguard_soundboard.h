/*
    Vanguard arcade machine soundboard emulator

    Copyright (c) 2011 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
 */
#ifndef VANGUARD_SOUNDBOARD_H_
#define VANGUARD_SOUNDBOARD_H_

#include <emu/emu_standard_machine.h>
#include <emu/emu_sample_player.h>

struct VanguardMusicChannel
{
    bool muted;
    bool one_shot;
    unsigned offset;
    unsigned offset_curr;
    unsigned offset_mask;
    unsigned char * rom;
    unsigned waveform_counter;
    unsigned waveform_step;
    int waveform[16];
    
    VanguardMusicChannel( unsigned char * rom );
    
    void reset();
    void setMuted( bool muted );
    void setOneShot( bool oneshot );
    void setOffset( unsigned o, unsigned m = 0xFF );
    void nextOffset();
    void setPitchFromCurrentRomOffset( unsigned sampling_rate );
    void setWaveform( int mask );
    void setWaveformData( const int * data, int scale );
            
    inline int getNextWaveformSample() {
        int result = waveform[ (waveform_counter >> 10) & 0x0F ];
        waveform_counter += waveform_step;    
        return result;
    }
};

struct VanguardSoundBoard
{
    unsigned ch_count_;
    VanguardMusicChannel * ch_[3];
    unsigned tone_period_;
    unsigned tone_counter_;
    unsigned tone_step_;
    
    VanguardSoundBoard( unsigned char * rom, unsigned num_channels );
    
    ~VanguardSoundBoard();
    
    void play( TMixerBuffer * mixer_buffer, unsigned samplesPerFrame, unsigned samplingRate );
    
    VanguardMusicChannel * channel( int index ) {
        return ch_[index];
    }
};

struct Hd38880_SimWithSamples {
    unsigned        speech_command_;
    unsigned        speech_data_;
    unsigned        speech_data_len_;
    unsigned        speech_data_ofs_;
    unsigned        speech_address_;
    int             speech_table_len_;
    const unsigned *speech_table_;
    TSamplePlayer * speech_samples_;
    
    Hd38880_SimWithSamples();
    
    void setSamples( int len, const unsigned * table, TSamplePlayer * samples );
    
    void reset();
    
    void write( unsigned char );
    
    void play( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate );
};

#endif // VANGUARD_SOUNDBOARD_H_
