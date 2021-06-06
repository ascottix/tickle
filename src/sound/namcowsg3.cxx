/*
    Namco custom waveform sound generator 3 (Pacman hardware)

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#include <string.h>

#include "namcowsg3.h"

NamcoWsg3::NamcoWsg3( unsigned masterClock )
{
    master_clock_ = masterClock;
    setSamplingRate( 0 );

    for( unsigned i=0; i<3; i++ ) {
        wave_offset_[i] = 0;
    }

    for( unsigned j=0; j<sizeof(sound_regs_); j++ ) {
        sound_regs_[j] = 0;
    }
}

void NamcoWsg3::setSoundPROM( const unsigned char * prom )
{
    memcpy( sound_prom_, prom, sizeof(sound_prom_) );

    for( int i=0; i<32*8; i++ ) {
        sound_wave_data_[i] = (int)*prom++; // 4-bit data
    }
}

void NamcoWsg3::getVoice( NamcoWsg3Voice * voice, int index ) const
{
    int base = 5*index;

    voice->waveform = sound_regs_[ 0x05 + base ] & 0x07;
    voice->volume = sound_regs_[ 0x15 + base ] & 0x0F;

    unsigned f;
    
    f =            (sound_regs_[ 0x14+base ] & 0x0F);
    f = (f << 4) | (sound_regs_[ 0x13+base ] & 0x0F);
    f = (f << 4) | (sound_regs_[ 0x12+base ] & 0x0F);
    f = (f << 4) | (sound_regs_[ 0x11+base ] & 0x0F);
    f = (f << 4);

    if( index == 0 ) { // The first voice has an extra 4-bit of data
        f |= (sound_regs_[ 0x10+base ] & 0x0F); 
    }

    voice->frequency = f;
}

/*
    Play and mix the sound voices into the specified buffer.
*/
void NamcoWsg3::playSound( int * buf, int len )
{
    NamcoWsg3Voice voice;

    for( int index=0; index<3; index++ ) {
        getVoice( &voice, index );
        if( voice.isActive() ) {
            unsigned offset = wave_offset_[index];
            unsigned offset_step = voice.frequency * resample_step_;
            int * wave_data = sound_wave_data_ + 32 * voice.waveform;

            for( int i=0; i<len; i++ ) {
                // Should be shifted right by 15, but we must also get rid
                // of the 10 bits used for decimals
                buf[i] += wave_data[(offset >> 25) & 0x1F] * (int) voice.volume;
                offset += offset_step;
            }

            wave_offset_[index] = offset;
        }
    }
}
