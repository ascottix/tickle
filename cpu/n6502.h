/*
    N6502 emulator

    Copyright (c) 2011 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef N6502_H_
#define N6502_H_

/**
    Environment for the N6502 CPU emulator.

    This class implements all input/output functions for the N6502 emulator class,
    that is it provides functions to access the system RAM, ROM and I/O ports.

    An object (instance) of this class corresponds to a system that has no RAM, ROM or 
    ports: users of the N6502 emulator should provide the desired behaviour by writing a
    descendant of this class that overrides the required functions.

    @author Alessandro Scotti
*/
class N6502Environment
{
public:
    /** 
        Constructor. 

        The object created by this base class corresponds to a system with no RAM, ROM or 
        ports.
    */
    N6502Environment() {
    }

    /** Destructor. */
    virtual ~N6502Environment() {
    }

    /**
        Reads one byte from memory at the specified address.

        The address parameter may contain any value, including
        values greater than 0xFFFF (64k), so if necessary
        the implementation must check for illegal values and
        handle them according to the implemented system 
        specifications.

        @param  addr    address of memory byte to read

        @return the content of the specified memory address
    */
    virtual unsigned char readByte( unsigned addr ) {
        return 0xFF;
    }

    /**
        Reads a 16 bit word from memory at the specified address.

        The address parameter may contain any value, including
        values greater than 0xFFFF (64k), so if necessary
        the implementation must check for illegal values and
        handle them according to the implemented system 
        specifications.

        The default implementation uses <i>readByte()</i>, so it is not
        strictly necessary to override this method.

        @see #readByte
    */
    virtual unsigned readWord( unsigned addr ) {
        return readByte(addr) | (unsigned)(readByte(addr+1) << 8);
    }

    /**
        Writes one byte to memory at the specified address.

        The address parameter may contain any value, including
        values greater than 0xFFFF (64k), so if necessary
        the implementation must check for illegal values and
        handle them according to the implemented system 
        specifications.

        @param  addr    address of memory byte to write
        @param  value   value to write at specified address
    */
    virtual void writeByte( unsigned addr, unsigned char value ) {
    }

    /**
        Writes a 16 bit word to memory at the specified address.

        The address parameter may contain any value, including
        values greater than 0xFFFF (64k), so if necessary
        the implementation must check for illegal values and
        handle them according to the specifications of the
        emulated system.

        The default implementation uses <i>writeByte()</i>, so it is not
        strictly necessary to override this method.

        <b>Note:</b> the low order byte is placed at the lowest address.

        @param  addr    address of memory word to write
        @param  value   16-bit value to write at specified address

        @see #writeByte
    */
    virtual void writeWord( unsigned addr, unsigned value ) {
        writeByte( addr, value & 0xFF );
        writeByte( addr+1, (value >> 8) & 0xFF );
    }

    /**
        Reads one byte from the specified port.

        <b>Note:</b> the port value is always between 00h and FFh included.

        @param  port    address of port to read

        @return the value of the specified port
    */
    virtual unsigned char readPort( unsigned port ) {
        return 0xFF;
    }

    /**
        Writes one byte from the specified port.

        <b>Note:</b> the port value is always between 00h and FFh included.

        @param  port    address of port to write
        @param  value   value to write to specified port
    */
    virtual void writePort( unsigned port, unsigned char value ) {
    }
};

/**
    N6502 CPU emulator.

    @author Alessandro Scotti
    @version 1.0
*/
class N6502
{
public:
    /** CPU flags. */
    enum Flags {
        Carry       = 0x01, // C
        Zero        = 0x02, // Z
        IrqDisabled = 0x04, // I
        DecimalMode = 0x08, // D
        Break       = 0x10, // B
        Bit5        = 0x20,
        Overflow    = 0x40, // V
        Sign        = 0x80  // N
    };

    /** Interrupt type */
    enum {
        Int_NMI = 0,    // Non Maskable Interrupt
        Int_IRQ         // Interrupt ReQuest (maskable)
    };

public:
    /** 8-bit register A (accumulator A). */
    unsigned char   A;
    /** 8-bit register X (index register X). */
    unsigned char   X;
    /** 8-bit register Y (index register Y). */
    unsigned char   Y;
    /** 8-bit flag register F (aka processor status P). */
    unsigned char   F;
    /** 8-bit stack pointer. */
    unsigned char   S;
    /** 16-bit program counter. */
    unsigned        PC;

    /** 
        Costructor.

        Creates an N6502 emulator associated to the specified <i>environment</i>.
        The <i>environment</i> provides functions to access memory and ports, that is
        it interfaces the emulated CPU with the other (emulated) hardware components.

        @see N6502Environment
    */
    N6502( N6502Environment & env );

    /** Destructor. */
    virtual ~N6502() {
    }

    /** Resets the CPU to its initial state. */
    virtual void reset();

    /** Executes one CPU instruction. */
    virtual void step();

    /**
        Runs the CPU for the specified number of cycles.

        Note that the number of CPU cycles performed by this function may be
        actually a little more than the value specified. If that happens then the
        function returns the number of extra cycles executed.

        @param cycles number of cycles the CPU must execute

        @return the number of extra cycles executed by the last instruction
    */
    virtual unsigned run( unsigned cycles );

    /** 
        Informs the CPU that an interrupt has occurred.

        @param type interrupt type
    */
    virtual void interrupt( int type );

    /** Returns the number of CPU cycles elapsed since last reset. */
    unsigned getCycles() const {
        return t_cycles_;
    }

    /** Sets the CPU cycle counter to the specified value. */
    void setCycles( unsigned value ) {
        t_cycles_ = value;
    }

protected:
    /* 
        Implementation of opcodes 0x00 to 0xFF.
    */
    void opcode_00();   // BRK
    void opcode_01();   // ORA (IND, X)
    void opcode_05();   // ORA ZP
    void opcode_06();   // ASL ZP
    void opcode_08();   // PHP
    void opcode_09();   // ORA IMM
    void opcode_0a();   // ASL A
    void opcode_0d();   // ORA ABS
    void opcode_0e();   // ASL ABS

    void opcode_10();   // BPL
    void opcode_11();   // ORA (IND), Y
    void opcode_15();   // ORA ZP, X
    void opcode_16();   // ASL ZP, X
    void opcode_18();   // CLC
    void opcode_19();   // ORA ABS, Y
    void opcode_1d();   // ORA ABS, X
    void opcode_1e();   // ASL ABS, X

    void opcode_20();   // JSR ABS
    void opcode_21();   // AND (IND, X)
    void opcode_24();   // BIT ZP
    void opcode_25();   // AND ZP
    void opcode_26();   // ROL ZP
    void opcode_28();   // PLP
    void opcode_29();   // AND IMM
    void opcode_2a();   // ROL A
    void opcode_2c();   // BIT ABS
    void opcode_2d();   // AND ABS
    void opcode_2e();   // ROL ABS

    void opcode_30();   // BMI
    void opcode_31();   // AND (IND), Y
    void opcode_35();   // AND ZP, X
    void opcode_36();   // ROL ZP, X
    void opcode_38();   // SEC
    void opcode_39();   // AND ABS, Y
    void opcode_3d();   // AND ABS, X
    void opcode_3e();   // ROL ABS, X

    void opcode_40();   // RTI
    void opcode_41();   // EOR (IND, X)
    void opcode_45();   // EOR ZP
    void opcode_46();   // LSR ZP
    void opcode_48();   // PHA
    void opcode_49();   // EOR IMM
    void opcode_4a();   // LSR A
    void opcode_4c();   // JMP ABS
    void opcode_4d();   // EOR ABS
    void opcode_4e();   // LSR ABS

    void opcode_50();   // BVC
    void opcode_51();   // EOR (IND), Y
    void opcode_55();   // EOR ZP, X
    void opcode_56();   // LSR ZP, X
    void opcode_58();   // CLI
    void opcode_59();   // EOR ABS, Y
    void opcode_5d();   // EOR ABS, X
    void opcode_5e();   // LSR ABS, X

    void opcode_60();   // RTS
    void opcode_61();   // ADC (IND, X)
    void opcode_65();   // ADC ZP
    void opcode_66();   // ROR ZP
    void opcode_68();   // PLA
    void opcode_69();   // ADC IMM
    void opcode_6a();   // ROR A
    void opcode_6c();   // JMP (ABS)
    void opcode_6d();   // ADC ABS
    void opcode_6e();   // ROR ABS

    void opcode_70();   // BVS
    void opcode_71();   // ADC (IND), Y
    void opcode_75();   // ADC ZP, X
    void opcode_76();   // ROR ZP, X
    void opcode_78();   // SEI
    void opcode_79();   // ADC ABS, Y
    void opcode_7d();   // ADC ABS, X
    void opcode_7e();   // ROR ABS, X

    void opcode_81();   // STA (IND, X)
    void opcode_84();   // STY ZP
    void opcode_85();   // STA ZP
    void opcode_86();   // STX ZP
    void opcode_88();   // DEY
    void opcode_8a();   // TXA
    void opcode_8c();   // STY ABS
    void opcode_8d();   // STA ABS
    void opcode_8e();   // STX ABS

    void opcode_90();   // BCC
    void opcode_91();   // STA (IND), Y
    void opcode_94();   // STY ZP, X
    void opcode_95();   // STA ZP, X
    void opcode_96();   // STX ZP, Y
    void opcode_98();   // TYA
    void opcode_99();   // STA ABS, Y
    void opcode_9a();   // TXS
    void opcode_9d();   // STA ABS, X

    void opcode_a0();   // LDY IMM
    void opcode_a1();   // LDA (IND, X)
    void opcode_a2();   // LDX IMM
    void opcode_a4();   // LDY ZP
    void opcode_a5();   // LDA ZP
    void opcode_a6();   // LDX ZP
    void opcode_a8();   // TAY
    void opcode_a9();   // LDA IMM
    void opcode_aa();   // TAX
    void opcode_ac();   // LDY ABS
    void opcode_ad();   // LDA ABS
    void opcode_ae();   // LDX ABS

    void opcode_b0();   // BCS
    void opcode_b1();   // LDA (IND), Y
    void opcode_b4();   // LDY ZP, X
    void opcode_b5();   // LDA ZP, X
    void opcode_b6();   // LDX ZP, Y
    void opcode_b8();   // CLV
    void opcode_b9();   // LDA ABS, Y
    void opcode_ba();   // TSX
    void opcode_bc();   // LDY ABS, X
    void opcode_bd();   // LDA ABS, X
    void opcode_be();   // LDX ABS, Y

    void opcode_c0();   // CPY IMM
    void opcode_c1();   // CMP (IND, X)
    void opcode_c4();   // CPY ZP
    void opcode_c5();   // CMP ZP
    void opcode_c6();   // DEC ZP
    void opcode_c8();   // INY
    void opcode_c9();   // CMP IMM
    void opcode_ca();   // DEX
    void opcode_cc();   // CPY ABS
    void opcode_cd();   // CMP ABS
    void opcode_ce();   // DEC ABS

    void opcode_d0();   // BNE
    void opcode_d1();   // CMP (IND), Y
    void opcode_d5();   // CMP ZP, X
    void opcode_d6();   // DEC ZP, X
    void opcode_d8();   // CLD
    void opcode_d9();   // CMP ABS, Y
    void opcode_dd();   // CMP ABS, X
    void opcode_de();   // DEC ABS, X

    void opcode_e0();   // CPX IMM
    void opcode_e1();   // SBC (IND, X)
    void opcode_e4();   // CPX ZP
    void opcode_e5();   // SBC ZP
    void opcode_e6();   // INC ZP
    void opcode_e8();   // INX
    void opcode_e9();   // SBC IMM
    void opcode_ea();   // NOP
    void opcode_ec();   // CPX ABS
    void opcode_ed();   // SBC ABS
    void opcode_ee();   // INC ABS

    void opcode_f0();   // BEQ
    void opcode_f1();   // SBC (IND), Y
    void opcode_f5();   // SBC ZP, X
    void opcode_f6();   // INC ZP, X
    void opcode_f8();   // SED
    void opcode_f9();   // SBC ABS, Y
    void opcode_fd();   // SBC ABS, X
    void opcode_fe();   // INC ABS, X

    /** Performs a relative branch if the specified condition is met. */
    void branchRelative( bool cond );

    /* Arithmetical operations */
    unsigned char aslByte( unsigned char op );
    void adcByte( unsigned char op );
    void andByte( unsigned char op );
    void cmpByte( unsigned char op1, unsigned char op2 );
    void eorByte( unsigned char op );
    unsigned char lsrByte( unsigned char op );
    void oraByte( unsigned char op );
    unsigned char rolByte( unsigned char op );
    unsigned char rorByte( unsigned char op );
    void sbcByte( unsigned char op );

    /* Addressing */
    unsigned getAddr( unsigned ofs );
    unsigned getAddrIndX();
    unsigned getAddrIndY();

    /* Stack */
    void pushByte( unsigned char b );
    void pushWord( unsigned w );
    
    unsigned char popByte();
    unsigned popWord();

    /** Sets the sign and zero flags according to the specified value. Returns the same value. */
    unsigned char setNZ( unsigned char b );

    /** Fetches the next 8-bit value at the program counter address. */
    unsigned char nextByte();

    /** Fetches the next 16-bit value at the program counter address. */
    unsigned nextWord();

private:
    typedef void (N6502::* OpcodeHandler)();

    typedef struct {
        OpcodeHandler   handler;
        unsigned        cycles;
    } OpcodeInfo;

    static OpcodeInfo   Opcode_[256];   // Opcode table

    unsigned cycles_;
    unsigned t_cycles_;
    N6502Environment &  env_;
};

#endif // N6502_H_
