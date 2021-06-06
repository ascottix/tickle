/*
    AY-3-8910/Z80 sound board emulator

    Copyright (c) 2004 Alessandro Scotti
*/
#include "z80_ay3_soundboard.h"

Z80_AY3_SoundBoard::Z80_AY3_SoundBoard( int numOfChips, unsigned chipClock, unsigned romSize, unsigned ramSize, unsigned ramStart )
{
    num_of_chips_ = numOfChips;
    chip_ = new AY_3_8910[ numOfChips ];
    for( int i=0; i<numOfChips; i++ ) {
        chip_[i].setClock( chipClock );
    }

    rom_size_ = romSize;
    rom_ = new unsigned char [ romSize ];

    ram_address_ = ramStart;
    ram_end_address_ = ramStart + ramSize;
    ram_size_ = ramSize;
    ram_ = new unsigned char [ ramSize ];

    cpu_ = new Z80( *this );

    interrupts_pending_ = 0;

    sampling_steps_ = 12;
}

Z80_AY3_SoundBoard::~Z80_AY3_SoundBoard()
{
    delete [] chip_;
    delete [] rom_;
    delete [] ram_;
    delete cpu_;
}

// Implementation of the Z80Environment interface
unsigned char Z80_AY3_SoundBoard::readByte( unsigned addr )
{
    addr &= 0xFFFF;

    if( addr < rom_size_ ) {
        return rom_[addr];
    }

    if( (addr >= ram_address_) && (addr < ram_end_address_) ) {
        return ram_[addr - ram_address_];
    }

    return onReadByte( addr );
}

void Z80_AY3_SoundBoard::writeByte( unsigned addr, unsigned char value )
{
    addr &= 0xFFFF;

    if( addr >= rom_size_ ) {
        if( (addr >= ram_address_) && (addr < ram_end_address_) ) {
            ram_[addr - ram_address_] = value;
        }
        else {
            onWriteByte( addr, value );
        }
    }
}

unsigned char Z80_AY3_SoundBoard::onReadByte( unsigned addr )
{
    return 0x00;
}

void Z80_AY3_SoundBoard::onWriteByte( unsigned addr, unsigned char value )
{
}

void Z80_AY3_SoundBoard::onAfterSamplingStep( unsigned step )
{
}

void Z80_AY3_SoundBoard::setPortData( unsigned index, unsigned char value )
{
    port_data_[index] = value;
}

void Z80_AY3_SoundBoard::onInterruptsEnabled()
{
    if( interrupts_pending_ > 0 ) {
        cpu_->interrupt( interrupt_queue_[ --interrupts_pending_ ] );
    }
}

void Z80_AY3_SoundBoard::run()
{
}

void Z80_AY3_SoundBoard::reset()
{
    cpu_->reset();
}

void Z80_AY3_SoundBoard::interrupt( unsigned char vector )
{
    if( ! cpu_->interrupt( vector ) ) {
        interrupt_queue_[ interrupts_pending_++ ] = vector;
    }
}

void Z80_AY3_SoundBoard::playSound( unsigned cpuCycles, TMixer * mixer, unsigned len, unsigned samplingRate )
{
    TMixerBuffer * mixerBuffer = mixer->getBuffer( chMono, len, 3*num_of_chips_ );
    int * dataBuffer = mixerBuffer->data();
    unsigned stepSize = len / sampling_steps_;
    unsigned cyclesPerStep = cpuCycles / sampling_steps_;

    for( unsigned i=1; i<sampling_steps_; i++ ) {
        cpu_->run( cyclesPerStep );

        for( unsigned j=0; j<num_of_chips_; j++ ) {
            chip_[j].playSound( dataBuffer, stepSize, samplingRate );
        }

        dataBuffer += stepSize;
        
        onAfterSamplingStep( i-1 );
    }

    // Play last step outside the loop to avoid round-off errors
    stepSize = len - stepSize*(sampling_steps_-1);

    cpu_->run( cyclesPerStep );

    for( unsigned j=0; j<num_of_chips_; j++ ) {
        chip_[j].playSound( dataBuffer, stepSize, samplingRate );
    }

    onAfterSamplingStep( sampling_steps_-1 );
}
