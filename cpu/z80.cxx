/*
    Z80 emulator

    Copyright (c) 1996-2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "z80.h"

/* Constructor */
Z80::Z80( Z80Environment & env )
    : env_( env )
{
    reset();
}

/* Resets the CPU */
void Z80::reset()
{
    PC = 0;         // Program counter is zero
    I = 0;          // Interrupt register cleared
    R = 0;          // Memory refresh register cleared
    iflags_ = 0;    // IFF1 and IFF2 cleared, IM0 enabled
    cycles_ = 0;
    t_cycles_ = 0;

    // There is no official documentation for the following!
    B = B1 = 0; 
    C = C1 = 0;
    D = D1 = 0; 
    E = E1 = 0;
    H = H1 = 0;
    L = L1 = 0;
    A = A1 = 0;
    F = F1 = 0;
    IX = 0;
    IY = 0;
    SP = 0xF000;
}

/* Executes one instruction */
void Z80::step()
{
    cycles_ = 0;

    // Update memory refresh register
    R = (R+1) & 0x7F; 

    if( iflags_ & Halted ) {
        // CPU is halted, do a NOP instruction
        cycles_ += OpInfo_[0].cycles; // NOP
    }
    else {
        // Get the opcode to execute
        unsigned op = fetchByte();

        // Update the cycles counter with the number of cycles for this opcode
        cycles_ += OpInfo_[ op ].cycles;

        // Execute the opcode handler
        (this->*(OpInfo_[ op ].handler))();

        // Update registers
        PC &= 0xFFFF; // Clip program counter
        SP &= 0xFFFF; // Clip stack pointer
    }
    
    t_cycles_ += cycles_;
}

/*
    Runs the CPU for the specified number of cycles.
*/
unsigned Z80::run( unsigned runCycles )
{
    int remainingCycles = (int) runCycles;

    // Execute instructions until the specified number of
    // cycles has elapsed
    while( remainingCycles > 0 ) {
        cycles_ = 0;
        
        // Update memory refresh register
        R = (R+1) & 0x7F; 

        if( iflags_ & Halted ) {
            // CPU is halted, do NOPs for the rest of cycles
            // (this may be off by a few cycles)
            cycles_ = remainingCycles;
        }
        else {
            // Get the opcode to execute
            unsigned op = fetchByte();

            // Update the cycles counter with the number of cycles for this opcode
            cycles_ += OpInfo_[ op ].cycles; 

            // Execute the opcode handler
            (this->*(OpInfo_[ op ].handler))();
        }
        
        t_cycles_ += cycles_;
        
        remainingCycles -= (int) cycles_;
    }

    // Update registers
    PC &= 0xFFFF; // Clip program counter
    SP &= 0xFFFF; // Clip stack pointer

    // Return the number of extra cycles executed
    return (unsigned) (-remainingCycles);
}

/* Interrupt */
bool Z80::interrupt( unsigned char data )
{
    bool result = false;
    
    // Execute interrupt only if interrupts are enabled
    if( iflags_ & IFF1 ) {
        cycles_ = 0;
        
        // Disable maskable interrupts and restart the CPU if halted
        iflags_ &= ~(IFF1 | IFF2 | Halted); 

        switch( getInterruptMode() ) {
        case 0:
            (this->*(OpInfo_[ data ].handler))();
            cycles_ += 11;
            break;
        case 1:
            callSub( 0x38 );
            cycles_ += 11;
            break;
        case 2:
            callSub( readWord( ((unsigned)I) << 8 | (data & 0xFE) ) );
            cycles_ += 19;
            break;
        }
        
        t_cycles_ += cycles_;

        result = true;
    }

    return result;
}

/* Non-maskable interrupt */
void Z80::nmi()
{
    cycles_ = 0;

    // Disable maskable interrupts but preserve IFF2 (that is a copy of IFF1),
    // also restart the CPU if halted
    iflags_ &= ~(IFF1 | Halted);

    callSub( 0x66 );

    cycles_ += 11;
    
    t_cycles_ += cycles_;
}
