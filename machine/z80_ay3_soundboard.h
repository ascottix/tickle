/*
    AY-3-8910/Z80 sound board emulator

    Copyright (c) 2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef Z80_AY3_SOUNDBOARD_H_
#define Z80_AY3_SOUNDBOARD_H_

#include <emu/emu_mixer.h>
#include <cpu/z80.h>
#include <sound/ay-3-8910.h>

struct Z80_AY3_SoundBoard : public Z80Environment
{
    Z80_AY3_SoundBoard( int numOfChips, unsigned chipClock, unsigned romSize, unsigned ramSize, unsigned ramStart );

    virtual ~Z80_AY3_SoundBoard();

    virtual void run();
    virtual void reset();
    virtual void interrupt( unsigned char vector );
    virtual void playSound( unsigned cpuCycles, TMixer * mixer, unsigned len, unsigned samplingRate );

    unsigned char getPortData( unsigned index ) {
        return port_data_[index];
    }

    void setPortData( unsigned index, unsigned char value );

    void setSamplingSteps( unsigned steps ) {
        sampling_steps_ = steps;
    }

    unsigned char * rom() {
        return rom_;
    }

    unsigned char * ram() {
        return ram_;
    }

    Z80 * cpu() {
        return cpu_;
    }

    AY_3_8910 * soundChip( int index ) {
        return &chip_[index];
    }

    // Implementation of the Z80Environment interface
    unsigned char readByte( unsigned addr );
    void writeByte( unsigned addr, unsigned char value );
    void onInterruptsEnabled();

protected:
    // Extension points for children
    virtual unsigned char onReadByte( unsigned addr );
    virtual void onWriteByte( unsigned addr, unsigned char value );
    virtual void onAfterSamplingStep( unsigned step );

    enum {
        PortDataSize = 8,
        InterruptQueueSize = 4
    };

    // Member variables
    unsigned char * rom_;
    unsigned char * ram_;
    unsigned rom_size_;
    unsigned ram_size_;
    unsigned ram_address_;
    unsigned ram_end_address_;
    unsigned num_of_chips_;
    unsigned char interrupt_queue_[InterruptQueueSize];
    unsigned char port_data_[PortDataSize];
    unsigned interrupts_pending_;
    AY_3_8910 * chip_;
    Z80 * cpu_;
    unsigned sampling_steps_;

    unsigned timer_clock_;
    bool interrupt_pending_;
};

#endif // Z80_AY3_SOUNDBOARD_H_
