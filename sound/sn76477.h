/*
    SN76477 sound chip emulation

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef SN76477_H_
#define SN76477_H_

enum {
    // Mixer
    snMixer_VCO = 0,
    snMixer_SLF = 1,
    snMixer_Noise = 2,
    snMixer_Noise_VCO = 3,
    snMixer_Noise_SLF = 4,
    snMixer_Noise_SLF_VCO = 5,
    snMixer_SLF_VCO = 6,
    snMixer_None = 7,
    // Envelope
    snEnv_VCO = 0,
    snEnv_MixerOnly = 1,
    snEnv_OneShot = 2,
    snEnv_AlternateVCO = 3
};

class SN76477
{
public:
    SN76477();

    /**
        Configures the Super Low Frequency oscillator.

        The SLF frequency in Hertz is given by 0.64 / (r*c). 
        
        @param r value of SLF resistor (pin 20) in Ohms
        @param c value of SLF capacitor (pin 21) in Farads
    */
    void setSLF( double r, double c );

    /**
        Configures the Super Low Frequency oscillator.

        As the SLF frequency is actually controlled by a resistor and a
        capacitor, this function uses a fixed built-in value for the capacitor
        and sets the resistor according to the specified frequency.
        
        @param f frequency in Hertz
    */
    void setSLF( double f ) {
        setSLF( 0.64 / (f * 0.000001), 0.000001 );
    }

    /**
        Configures the Voltage Controlled Oscillator.

        The VCO minimum frequency in Hertz is given by 0.64 / (r*c),
        the maximum frequency is about 10 times the minimum frequency.

        Increasing the voltage applied to pin 16 (frequency control) decreases
        the VCO frequency.

        Increasing the voltage applied to pin 19 (pitch control) increases the 
        VCO duty cycle.

        The duty cycle expressed as a percentual of the wave period is:
        
            50 * (voltage pin 19)/(voltage pin 16).

        @param r value of VCO resistor (pin 18) in Ohms
        @param c value of VCO capacitor (pin 17) in Farads
        @param p pitch control (pin 19, Volts): 0 means pin is high and duty cycle is 50%
        @param f frequency control (pin 16, Volts)
    */
    void setVCO( double r, double c, double p, double f );

    void setVCO( double f ) {
        setVCO( 0.64 / (f * 0.000001), 0.000001, 0, 0 );
    }

    /**
        Selects the input source for the VCO.

        Note: this is pin 22 of the chip.

        @param source 0 for external control (pin 16), 1 for internal control (SLF)
    */
    void setVCO_Select( int source );

    /**
        Sets the output of sound generator.

        Mixer is selected on chip by pins 25, 26 and 27. It can assume
        one of the following values:
        <PRE>
            <B>
            Value   Output              Pin27/C Pin25/B Pin26/A
            </B>
            0       VCO                 L       L       L
            1       SLF                 L       L       H
            2       Noise               L       H       L
            3       VCO/Noise           L       H       H
            4       SLF/Noise           H       L       L
            5       SLF/VCO/Noise       H       L       H
            6       SLF/VCO             H       H       L
            7       none                H       H       H
        </PRE>

        @param mixer output of sound generator
    */
    void setMixer( unsigned mixer );

    /**
        Sets the one-shot impulse duration.

        The one-shot impulse lasts for about 0.8 * r * c seconds.

        @param r value of one-shot resistor (pin 24) in Ohms
        @param c value of one-shot capacitor (pin 23) in Farads
    */
    void setOneShot( double r, double c );

    /** 
        Enables or disables sound output.

        Note: this is the inverse of pin 9 (which is called "output inhibit").

        @param enabled true to enable sound, false otherwise
    */
    void enableOutput( bool enabled );
    
    /**
        Returns true if the output is enabled, false otherwise.
    */
    bool isOutputEnabled() {
        return enabled_;
    }

    /**
        Sets the envelope applied to the mixer output.

        Envelope is selected on chip by pins 1 and 28. It can assume 
        one of the following values:
        <PRE>
            <B>
            Value   Output          Pin1/1  Pin28/2
            </B>
            0       VCO             L       L
            1       Mixer only      L       H
            2       One-shot        H       L
            3       VCO alternate   H       H
        </PRE>

        VCO alternate is same as VCO but skips every other VCO pulse.

        @param envelope envelope applied to mixer output
    */
    void setEnvelope( int envelope );

    /**
        Sets the envelope attack and decay periods.

        Attack time in seconds is approximately r_attack * c,
        decay time in seconds is approximately r_decay * c.

        @param r_attack attack resistor (pin 10) in Ohms
        @param r_decay decay resistor (pin 7) in Ohms
        @param c attack/decay capacitor (pin 8) in Farads
    */
    void setEnvelopeControl( double r_attack, double r_decay, double c );

    /**
        Sets the noise filter.

        The noise generator passes thru the noise filter before going into the mixer.
        The 3-dB rolloff point of this low-pass filter is controlled by a resistor and
        a capacitor according to the following equation:

        3-dB frequency (Hz) = 1.28 / (r*c) at Vreg = 5 V.

        The resistor value should be 7.5 kOhm minimum.

        @param r noise low-pass filter resistor (pin 5) in Ohms
        @param c noise low-pass filter capacitor (pin 6) in Farads
    */
    void setNoiseFilter( double r, double c );

    /**
        Sets the interal noise clock generator speed.

        The nominal value of the resistor is 47 kOhm, smaller values give faster
        clock rates, larger values (up to a 100 kOhm max) give slower clock rates.

        @param r noise click resistor (pin 4) in Ohms
    */
    void setNoiseClock( double r );

    /**
        Sets the frequency of the noise clock generator.

        External clock signal should be a square wave applied to pin 3. When external clock is 
        used, internal noise clock should be disabled by connecting pin 4 to a high logic voltage level.

        @param freq noise clock frequency in Hertz (same as pin 3 connected to an external clock source)
    */
    void setExternalNoiseClock( double freq );

    /**
        Sets the amplifier output level.

        If resistors are connected as in the default model (Rf between pins 12 and 13, 
        Rg at pin 11) then the output is Vout = 3.4 * Rf / Rg. At any rate,
        the output cannot exceed 2.5V.

        Note: this functions computes a max output value according to the above
        formula and then calls setAmplifier(int).

        @param rf feedback resistor (pin 12) in Ohms
        @param rg amplitude control resistor (pin 11) in Ohms
    */
    void setAmplifier( double rf, double rg );

    /**
        Sets the amplifier output level.

        @param max max output value, from 0 to 255
    */
    void setAmplifier( int max );

    /** */
    void playSound( int * buffer, int len, unsigned samplingRate );

    /** Returns the noise frequency set by specified resistor (at pin 4). */
    static double getNoiseFreqFromRes( double r );

private:
    enum {
        ufSLF       = 0x01,
        ufVCO_RC    = 0x02,
        ufVCO_PF    = 0x04,
        ufOneShot   = 0x08,
        ufEnvelope  = 0x10,
        ufNoise     = 0x20,
        ufUpdateAll = 0xFFFF
    };

    void refreshParameters();

    double slf_r_;
    double slf_c_;
    double vco_r_;
    double vco_c_;
    double vco_f_;
    double vco_p_;
    double oneshot_r_;
    double oneshot_c_;
    double env_r_attack_;
    double env_r_decay_;
    double env_c_;
    double noise_r_;
    double noise_c_;
    double amp_rf_;
    double amp_rg_;
    bool enabled_;
    int vco_select_;
    unsigned mixer_;
    int envelope_;
    int volume_;
    unsigned update_flags_;
    unsigned sampling_rate_;

    unsigned envelope_mask_[4];
    unsigned envelope_value_;
    double   envelope_y_;
    double   envelope_a_;
    double   envelope_b_;
    double   envelope_coeff_a_[2];
    double   envelope_coeff_b_[2];
    unsigned noise_half_period_;
    unsigned noise_offset_;
    unsigned noise_shift_register_;
    unsigned noise_output_;
    unsigned noise_mixer_mask_;
    double   noise_freq_;
    double   noise_rc_a_;
    double   noise_rc_b_;
    double   noise_rc_y_;
    unsigned oneshot_period_;
    unsigned oneshot_offset_;
    unsigned slf_half_period_;
    unsigned slf_offset_;
    unsigned slf_output_;
    unsigned slf_mixer_mask_;
    unsigned vco_half_period_[2];
    unsigned vco_half_period_index_;
    unsigned vco_min_period_;
    unsigned vco_max_period_;
    unsigned vco_duty_cycle_; // Percentage, rescaled from 0..100 to 0..512
    unsigned vco_offset_;
    unsigned vco_output_;
    unsigned vco_mixer_mask_;
    unsigned vco_alternate_index_;
    int      volume_table_[256];
};

#endif // SN76477_H_
