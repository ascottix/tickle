/*
    SN76477 sound chip emulation

    Copyright (c) 2004-2010,2011 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include <math.h>

#include "sn76477.h"

const unsigned LFSR_MASK = 0x30009;
const double MaxExternalVcoControl = 2.35;

SN76477::SN76477()
{
    sampling_rate_ = 0;
    enabled_ = false;
    env_c_ = 0; // Not connected (disables attack and decay)
    noise_shift_register_ = 1; // Any (17-bits) nonzero value
    noise_output_ = 0;
    oneshot_period_ = 0;
    slf_output_ = 0;
    vco_alternate_index_ = 3;
    vco_half_period_index_ = 0;
    vco_output_ = 0;
    update_flags_ = ufUpdateAll;

    setAmplifier( 1.0, 3.4 ); // Set default volume to slightly less than half
    setMixer( snMixer_None ); // Disable output

    // Setup envelopes
    envelope_mask_[snEnv_VCO] = 0x00; // Toggled by VCO update
    envelope_mask_[snEnv_MixerOnly] = 0xFF; // Always set
    envelope_mask_[snEnv_OneShot] = 0; // Will be set by enabled bit
    envelope_mask_[snEnv_AlternateVCO] = 0; // Will be set by VCO update
    envelope_ = snEnv_MixerOnly;
}

void SN76477::setNoiseFilter( double r, double c )
{
    noise_r_ = r;
    noise_c_ = c;
    update_flags_ |= ufNoise;
}

double SN76477::getNoiseFreqFromRes( double r ) {
    r /= 1000.0;
    
    double f = 783000.0 / pow(r, 0.9);
    
    if( r <= 100 ) {
        f = 759395.0 / pow(r, 0.887); // More accurate for this range
    }
    
    return f;
}

void SN76477::setNoiseClock( double r )
{
    setExternalNoiseClock( getNoiseFreqFromRes(r) );
}

void SN76477::setExternalNoiseClock( double freq )
{
    noise_freq_ = freq;
    update_flags_ |= ufNoise;
}

void SN76477::setMixer( unsigned mixer ) 
{
    static unsigned MixerMask[8] = {
        0xFFFF00,   // VCO
        0xFF00FF,   // SLF
        0x00FFFF,   // Noise
        0x00FF00,   // VCO/Noise
        0x0000FF,   // SLF/Noise
        0x000000,   // SLF/VCO/Noise
        0xFF0000,   // SLF/VCO
        0xFFFFFF    // None
    };

    mixer_ = mixer & 0x07;

    unsigned m = MixerMask[mixer_];

    vco_mixer_mask_ = m & 0xFF;
    slf_mixer_mask_ = (m >> 8) & 0xFF;
    noise_mixer_mask_ = (m >> 16) & 0xFF;
}

void SN76477::setSLF( double r, double c )
{
    slf_r_ = r;
    slf_c_ = c;
    update_flags_ |= ufSLF;
}

void SN76477::refreshParameters()
{
    if( update_flags_ & ufSLF ) {
        slf_half_period_ = (unsigned) (0.5 + (slf_r_ * slf_c_ * sampling_rate_ / 0.64) / 2);
        slf_offset_ = 0;
    }

    if( update_flags_ & ufVCO_RC ) {
        vco_max_period_ = (unsigned) (0.5 + vco_r_ * vco_c_ * sampling_rate_ / 0.64); // Minimum frequency
        vco_min_period_ = vco_max_period_ / 10;
        vco_offset_ = 0;
    }

    if( update_flags_ & ufVCO_PF ) {
        // Note: duty cycle percentage is rescaled to 512
        if( (vco_f_ != 0) && (vco_p_ != 0) ) {
            vco_duty_cycle_ = (unsigned) ((vco_p_ * 256.0) / vco_f_);

            // Clip duty cycle between 18% and 50%
            if( vco_duty_cycle_ > 256 ) {
                vco_duty_cycle_ = 256;
            }
            else if( vco_duty_cycle_ < 92 ) {
                vco_duty_cycle_ = 92;
            }
        }   
        else {
            vco_duty_cycle_ = 256; 
        }

        if( ! vco_select_ ) {
            unsigned t = vco_min_period_ + (unsigned) (vco_f_ * (vco_max_period_-vco_min_period_) / MaxExternalVcoControl);

            if( t > vco_max_period_ ) {
                t = vco_max_period_;
            }

            vco_half_period_[0] = t * vco_duty_cycle_ / 512;
            vco_half_period_[1] = t - vco_half_period_[0];
        }
    }

    if( update_flags_ & ufOneShot ) {
        // Compute one-shot duration in samples
        oneshot_period_ = (unsigned) (0.8 * oneshot_r_ * oneshot_c_ * sampling_rate_);
    }

    if( update_flags_ & ufEnvelope ) {
        // Initialize filter so that output is unchanged
        envelope_coeff_b_[0] = 0;
        envelope_coeff_b_[1] = 0;

        // Apply filters that are correctly defined
        if( (env_r_attack_ > 0) && (env_c_ > 0) ) {
            double d = env_r_attack_ * env_c_ * sampling_rate_;
            
            envelope_coeff_b_[1] = exp( -1/d );
        }

        if( (env_r_decay_ > 0) && (env_c_ > 0) ) {
            double d = env_r_decay_ * env_c_ * sampling_rate_;
            
            envelope_coeff_b_[0] = exp( -1/d );
        }

        envelope_coeff_a_[0] = 1 - envelope_coeff_b_[0];
        envelope_coeff_a_[1] = 1 - envelope_coeff_b_[1];
    }

    if( update_flags_ & ufNoise ) {
        noise_half_period_ = (unsigned) (0.5 + (sampling_rate_ / noise_freq_) / 2);

        double d = noise_r_ * noise_c_ * sampling_rate_;
        
        noise_rc_b_ = exp( -1/d );
        noise_rc_a_ = 1 - noise_rc_b_;
        noise_rc_y_ = 0;
    }

    update_flags_ = 0;
}

void SN76477::setVCO( double r, double c, double p, double f )
{
    if( (r != vco_r_) || (c != vco_c_) ) {
        vco_r_ = r;
        vco_c_ = c;
        update_flags_ |= ufVCO_RC;
    }

    if( (vco_f_ != f) || (vco_p_ != p) ) {
        vco_f_ = f;
        vco_p_ = p;
        update_flags_ |= ufVCO_PF;
    }
}

void SN76477::setVCO_Select( int source )
{
    if( vco_select_ != source ) {
        vco_select_ = source;
        update_flags_ |= ufVCO_PF;
    }
}

void SN76477::setOneShot( double r, double c )
{
    oneshot_r_ = r;
    oneshot_c_ = c;
    update_flags_ |= ufOneShot;
}

void SN76477::setEnvelope( int envelope )
{
    envelope_ = envelope;
}


void SN76477::setEnvelopeControl( double r_attack, double r_decay, double c )
{
    env_r_attack_ = r_attack;
    env_r_decay_ = r_decay;
    env_c_ = c;
    update_flags_ |= ufEnvelope;
}

void SN76477::setAmplifier( int max )
{
    if( max > 255 ) {
        max = 255;
    }

    for( int i=0; i<256; i++ ) {
        volume_table_[i] = (int) (i * max / 255);
    }
}

void SN76477::setAmplifier( double rf, double rg )
{
    amp_rf_ = rf;
    amp_rg_ = rg;

    setAmplifier( (int) ((3.4 * rf / rg) * 255 / 2.5) );
}

void SN76477::enableOutput( bool enabled )
{
    if( enabled_ != enabled ) {
        enabled_ = enabled;

        if( enabled_ ) {
            envelope_y_ = 0; // No previous output

            // Trigger one-shot
            oneshot_offset_ = oneshot_period_;
            envelope_mask_[snEnv_OneShot] = 0xFF;
        }
    }
}

void SN76477::playSound( int * buffer, int len, unsigned samplingRate )
{
    if( ! enabled_ ) {
        return;
    }

    // Refresh parameters if something has changed since last call
    if( sampling_rate_ != samplingRate ) {
        sampling_rate_ =  samplingRate;
        update_flags_ = ufUpdateAll;
    }

    if( update_flags_ ) {
        refreshParameters();
    }

    // Play the sound
    while( len > 0 ) {
        // Update SLF
        if( slf_offset_ >= slf_half_period_ ) {
            // Invert output
            slf_offset_ = 0;
            slf_output_ ^= 0xFF;
        }

        // Update VCO
        if( vco_select_ ) {
            // Internal control: SLF modulates VCO frequency
            unsigned o = slf_output_ ? slf_offset_ : slf_half_period_ - slf_offset_;
            unsigned t = vco_min_period_ + (unsigned) (o * (vco_max_period_-vco_min_period_) / slf_half_period_);

            vco_half_period_[0] = t * vco_duty_cycle_ / 512;
            vco_half_period_[1] = t - vco_half_period_[0];
        }

        if( vco_offset_ >= vco_half_period_[vco_half_period_index_] ) {
            // Invert output
            vco_offset_ = 0;
            vco_output_ ^= 0xFF;
            vco_half_period_index_ ^= 1;

            // Update envelope masks
            envelope_mask_[snEnv_VCO] ^= 0xFF;

            if( vco_alternate_index_ ) {
                envelope_mask_[snEnv_AlternateVCO] = 0x00;
                vco_alternate_index_--;
            }
            else {
                envelope_mask_[snEnv_AlternateVCO] = 0xFF;
                vco_alternate_index_ = 3;
            }
        }

        // Update noise
        if( noise_offset_ >= noise_half_period_ ) {
            if( noise_shift_register_ & 1 ) {
                noise_shift_register_ ^= LFSR_MASK;
                noise_output_ = 0xFF;
            }
            else {
                noise_output_ = 0x00;
            }

            noise_shift_register_ >>= 1;
            noise_offset_ = 0;
        }
        
        // Bump offsets
        slf_offset_++;
        vco_offset_++;
        noise_offset_++;

        // Update one-shot timer
        if( oneshot_offset_ ) {
            if( --oneshot_offset_ == 0 ) {
                envelope_mask_[snEnv_OneShot] = 0x00;
            }
        }

        // Compute envelope
        unsigned e = envelope_mask_[envelope_];

        if( e ^ envelope_value_ ) {
            // Envelope changed, select attack/decay coefficients accordingly
            envelope_a_ = envelope_coeff_a_[e & 0x01];
            envelope_b_ = envelope_coeff_b_[e & 0x01];
            envelope_value_ = e;
        }

        // Apply attack/decay filter
        envelope_y_ = envelope_a_ * e + envelope_b_ * envelope_y_;

        e = (unsigned) envelope_y_;

        // Mix SLF, VCO and Noise according to mixer settings
        unsigned s = (vco_output_ | vco_mixer_mask_) & (slf_output_ | slf_mixer_mask_) & (noise_output_ | noise_mixer_mask_);

        // Combine mixer output with envelope and write to output buffer
        *buffer++ = volume_table_[s & e];

        len--;
    }
}
