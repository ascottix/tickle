/*
    N6502 emulator

    Copyright (c) 2011 Alessandro Scotti
*/
#include "n6502.h"

N6502::N6502( N6502Environment & env )
    : env_( env )
{
    reset();
}

void N6502::reset()
{
    A = 0xAA;
    X = 0;
    Y = 0;
    F = Zero | IrqDisabled | Break;
    S = 0xFD;
    PC = env_.readWord( 0xFFFC );

    cycles_ = 0;
    t_cycles_ = 0;
}

void N6502::step()
{
    cycles_ = 0;

    unsigned op = nextByte();

    // Execute
    cycles_ += Opcode_[ op ].cycles;

    if( Opcode_[ op ].handler ) {
        (this->*(Opcode_[ op ].handler))();
    }
    
    t_cycles_ += cycles_;

    PC &= 0xFFFF;
}

unsigned N6502::run( unsigned runCycles )
{
    int remainingCycles = (int) runCycles;

    // Execute instructions until the specified number of cycles has elapsed
    while( remainingCycles > 0 ) {
        cycles_ = 0;
    
        unsigned op = env_.readByte( PC++ );

        cycles_ += Opcode_[ op ].cycles;

        if( Opcode_[ op ].handler ) {
            (this->*(Opcode_[ op ].handler))();
        }
        
        t_cycles_ += cycles_;
        
        remainingCycles -= (int) cycles_;
    }

    // Update registers
    PC &= 0xFFFF; // Clip program counter

    // Return the number of extra cycles executed
    return (unsigned) (-remainingCycles);
}

void N6502::interrupt( int type )
{
    if( (type == Int_NMI) || ((F & IrqDisabled) == 0) ) {
        cycles_ = 7;

        pushWord( PC );
        pushByte( F & ~Break );

        F &= ~DecimalMode;

        if( type == Int_NMI ) {
            PC = 0xFFFA;
        }
        else {
            PC = 0xFFFE;
            F |= IrqDisabled;
        }

        PC = nextWord();
        
        t_cycles_ += cycles_;
    }
}
