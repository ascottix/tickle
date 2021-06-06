/*
    YM2149 / AY-3-8910 sound chip emulator

    Copyright (c) 2004-2011 Alessandro Scotti
*/
#include <math.h>

#include "ym2149.h"

/*
    This mask is used to update the 17 bit Linear Feedback Shift Register
    used by the noise generator. The tap sequence is (17,3,0) and in the
    mask an extra bit has been added so that a 1 is automatically shifted
    in the correct position when the mask is "xor"ed, which saves an "or".
*/
const unsigned LFSR_MASK = 0x30009;

const unsigned MaxEnvelopeLevel = YM2149::EnvelopeSteps-1;

void YM2149::initializeVolumeTable() {
    for( int i=0; i<EnvelopeSteps; i++ ) {
        double v = 0.0055244 * pow( sqrt(2), (double)(i-1)/2.0 );
        EnvelopeVolumeTable[i] = (int) (v * 255.0);
    }
    EnvelopeVolumeTable[0] = 0;
    
    for( int i=0; i<16; i++ ) {
        VolumeTable[i] = EnvelopeVolumeTable[i*2+1];
    }
    VolumeTable[0] = 0;
}

unsigned YM2149::getEnvelopeTableEntryForLevel( unsigned level )
{
    // Normalize the amplitude and replicate the entry for all three channels
    level = EnvelopeVolumeTable[level];

    return level | (level << 8) | (level << 16);
}

void YM2149::initializeEnvelopeTable() {
    unsigned * env = EnvelopeTable;

    for( unsigned shape=0x00; shape<=0x0F; shape++ ) {
        unsigned alternate = shape & 0x02;
        
        // Set initial level according to the attack value
        unsigned level = shape & 0x04 ? 0 : MaxEnvelopeLevel;

        for( int j=0; j<3; j++ ) {
            // Ramp up or down
            if( level == 0 ) {
                for( int i=0; i<=(int)MaxEnvelopeLevel; i++ ) *env++ = getEnvelopeTableEntryForLevel((unsigned)i);
                level = MaxEnvelopeLevel;
            }
            else {
                for( int i=MaxEnvelopeLevel; i>=0; i-- ) *env++ = getEnvelopeTableEntryForLevel((unsigned)i);
                level = 0;
            }

            // Prepare for second and third sections (repeatable)
            if( shape & 0x08 ) {
                // Continue
                if( shape & 0x01 ) {
                    // Hold, set remainder to hold level and exit
                    if( alternate ) {
                        level = MaxEnvelopeLevel-level;
                    }
                    for( int i=0; i<EnvelopeSteps*2; i++ ) *env++ = getEnvelopeTableEntryForLevel(level);
                    break;
                }
                else {
                    // Shape not held, set level for next repetition
                    if( ! alternate ) {
                        level = MaxEnvelopeLevel-level;
                    }
                }
            }
            else {
                // Do not continue, set remainder to zero and exit
                for( int i=0; i<EnvelopeSteps*2; i++ ) *env++ = 0;
                break;
            }
        }
    }
}

// Constructor
YM2149::YM2149( unsigned clock )
{
    setClock( clock );
    
    initializeVolumeTable();
    initializeEnvelopeTable();

    // Reset registers
    reset();
}

void YM2149::setClock( unsigned clock )
{
    // Set master clock
    if( clock < MinClock ) clock = MinClock;
    if( clock > MaxClock ) clock = MaxClock;
    masterClock_ = clock;

    // Set reference clock for tone, noise and envelope
    soundClock_ = masterClock_ / 16;
}

void YM2149::reset()
{
    int i;

    address_latch_ = 0;
    tone_value_ = 0;
    noise_value_ = 0;
    noise_shift_register_ = 1; // Any non-zero value will do

    for( i=0; i<NumRegisters; i++ ) {
        reg_[i] = 0;
    }

    for( i=0; i<NumChannels+2; i++ ) {
        tone_counter_[i] = 0;
    }

    envelope_shape_counter_ = 0;
}

void YM2149::setRegister( unsigned index, unsigned char value ) 
{
    if( index < NumRegisters ) {
        reg_[index] = value;

        if( index == EnvelopeShape ) envelope_shape_counter_ = 0;
    }
}

unsigned YM2149::getChannelPeriod( unsigned channel ) const
{
    if( channel <= 2 )
        return reg_[channel*2] | (((unsigned)(reg_[channel*2+1] & 0x0F)) << 8);
    else if( channel == 3 )
        return reg_[NoisePeriod] & 0x1F;
    else if( channel == 4 )
        return getEnvelopePeriod();
    else
        return 0;
}

/*
    To see how sound is generated, let's consider a single channel
    for the moment.

    The tone generator outputs a square wave, i.e. a sequence of alternating
    0 and 1, but if the channel is disabled then it always outputs 1. This
    can be obtained by "or"ing the output of the generator with the disable bit.

    The noise generator works identically to the tone generator but there is only 
    one of it, which is "and"ed with the output of all three tone generators.

    Eventually the resulting value is modulated with the channel amplitude or
    the amplitude coming from the envelope generator.

    The following code closely follows the above algorithm, which I think make
    it very easy to follow. The only thing to note is that the output of all
    channels is stored into a single variable (using 8 bits per channel) so that
    masking operations can be performed in parallel on all channels.

*/
void YM2149::playSound( int * buffer, int len, unsigned samplingRate )
{
    unsigned step = (soundClock_ << 10) / samplingRate;

    // Prepare the channel mixer masks that will be combined with the square wave
    unsigned tone_mixer_mask = 0x000000;

    if( reg_[Mixer] & MixerToneA ) tone_mixer_mask |= 0x0000FF;
    if( reg_[Mixer] & MixerToneB ) tone_mixer_mask |= 0x00FF00;
    if( reg_[Mixer] & MixerToneC ) tone_mixer_mask |= 0xFF0000;

    // Prepare the noise mixer mask for combining tone and noise generators
    unsigned noise_mixer_mask = 0x000000;

    if( reg_[Mixer] & MixerNoiseA ) noise_mixer_mask |= 0x0000FF;
    if( reg_[Mixer] & MixerNoiseB ) noise_mixer_mask |= 0x00FF00;
    if( reg_[Mixer] & MixerNoiseC ) noise_mixer_mask |= 0xFF0000;

    // Compute the half period (in fixed point format) for each channel,
    // including noise and envelope
    unsigned half_period[5];

    for( int i=0; i<5; i++ ) {
        half_period[i] = getChannelPeriod(i) << 9; // Fixed point uses 10 bits for decimals, but here it's divided by 2
        if( i == 4 ) half_period[i] >>= 1; // Compared to the AY3-8910, YM2149's envelope has twice the steps and its counter goes twice as fast
        if( tone_counter_[i] >= half_period[i] ) {
            tone_counter_[i] = 0;
        }
    }

    // Prepare the amplitude and envelope mixer mask
    unsigned * envelope = EnvelopeTable + (getEnvelopeShape() * EnvelopeSteps * 3);
    unsigned envelope_mixer_mask = 0;
    unsigned tone_volume = 0;

    for( int c=2; c>=0; c-- ) {
        envelope_mixer_mask <<= 8;
        tone_volume <<= 8;
        if( isChannelUsingEnvelope(c) ) {
            envelope_mixer_mask |= 0xFF;
        }
        else {
            tone_volume |= VolumeTable[ getChannelVolume(c) ];
        }
    }

    // Play the sounds
    while( len > 0 ) {
        // Channel A
        tone_counter_[0] += step;
        if( tone_counter_[0] >= half_period[0] ) {
            tone_counter_[0] -= half_period[0];
            tone_value_ ^= 0x0000FF;
        }
        
        // Channel B
        tone_counter_[1] += step;
        if( tone_counter_[1] >= half_period[1] ) {
            tone_counter_[1] -= half_period[1];
            tone_value_ ^= 0x00FF00;
        }
        
        // Channel C
        tone_counter_[2] += step;
        if( tone_counter_[2] >= half_period[2] ) {
            tone_counter_[2] -= half_period[2];
            tone_value_ ^= 0xFF0000;
        }
        
        // Noise
        tone_counter_[3] += step;
        if( tone_counter_[3] >= half_period[3] ) {
            tone_counter_[3] -= half_period[3];
            if( noise_shift_register_ & 1 ) {
                noise_shift_register_ ^= LFSR_MASK;
                noise_value_ = 0xFFFFFF;
            }
            else {
                noise_value_ = 0x000000;
            }
            noise_shift_register_ >>= 1;
        }

        // Envelope
        tone_counter_[4] += step;
        if( tone_counter_[4] >= half_period[4] ) {
            tone_counter_[4] -= half_period[4];
            envelope_shape_counter_++;
            if( envelope_shape_counter_ == (EnvelopeSteps*3) ) {
                envelope_shape_counter_ = EnvelopeSteps;
            }
        }

        // Get output of tone and noise generators
        unsigned sample = (tone_value_ | tone_mixer_mask) & (noise_value_ | noise_mixer_mask);

        // Modulate with amplitude
        sample &= tone_volume | (envelope_mixer_mask & envelope[envelope_shape_counter_]);

        // Write all three channels in the output buffer
        *buffer += (sample      ) & 0xFF;
        *buffer += (sample >>  8) & 0xFF;
        *buffer += (sample >> 16);

        buffer++;

        len--;
    }
}
