/*
    N6502 emulator

    Copyright (c) 2011 Alessandro Scotti
*/
#include "n6502.h"

N6502::OpcodeInfo N6502::Opcode_[256] = {
    // Opcode handler, cycles, mnemonic
    //
    { &N6502::opcode_00,  7},   // BRK
    { &N6502::opcode_01,  6},   // ORA (IND, X)
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_05,  3},   // ORA ZP
    { &N6502::opcode_06,  5},   // ASL ZP
    { 0, 2 },
    { &N6502::opcode_08,  3},   // PHP
    { &N6502::opcode_09,  2},   // ORA IMM
    { &N6502::opcode_0a,  2},   // ASL A
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_0d,  4},   // ORA ABS
    { &N6502::opcode_0e,  6},   // ASL ABS
    { 0, 2 },

    { &N6502::opcode_10,  2},   // BPL
    { &N6502::opcode_11,  5},   // ORA (IND), Y
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_15,  4},   // ORA ZP, X
    { &N6502::opcode_16,  6},   // ASL ZP, X
    { 0, 2 },
    { &N6502::opcode_18,  2},   // CLC
    { &N6502::opcode_19,  4},   // ORA ABS, Y
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_1d,  4},   // ORA ABS, X
    { &N6502::opcode_1e,  7},   // ASL ABS, X
    { 0, 2 },

    { &N6502::opcode_20,  6},   // JSR ABS
    { &N6502::opcode_21,  6},   // AND (IND, X)
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_24,  3},   // BIT ZP
    { &N6502::opcode_25,  3},   // AND ZP
    { &N6502::opcode_26,  5},   // ROL ZP
    { 0, 2 },
    { &N6502::opcode_28,  4},   // PLP
    { &N6502::opcode_29,  2},   // AND IMM
    { &N6502::opcode_2a,  2},   // ROL A
    { 0, 2 },
    { &N6502::opcode_2c,  4},   // BIT ABS
    { &N6502::opcode_2d,  4},   // AND ABS
    { &N6502::opcode_2e,  6},   // ROL ABS
    { 0, 2 },

    { &N6502::opcode_30,  2},   // BMI
    { &N6502::opcode_31,  5},   // AND (IND), Y
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_35,  4},   // AND ZP, X
    { &N6502::opcode_36,  6},   // ROL ZP, X
    { 0, 2 },
    { &N6502::opcode_38,  2},   // SEC
    { &N6502::opcode_39,  4},   // AND ABS, Y
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_3d,  4},   // AND ABS, X
    { &N6502::opcode_3e,  7},   // ROL ABS, X
    { 0, 2 },

    { &N6502::opcode_40,  6},   // RTI
    { &N6502::opcode_41,  6},   // EOR (IND, X)
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_45,  3},   // EOR ZP
    { &N6502::opcode_46,  5},   // LSR ZP
    { 0, 2 },
    { &N6502::opcode_48,  3},   // PHA
    { &N6502::opcode_49,  2},   // EOR IMM
    { &N6502::opcode_4a,  2},   // LSR A
    { 0, 2 },
    { &N6502::opcode_4c,  3},   // JMP ABS
    { &N6502::opcode_4d,  4},   // EOR ABS
    { &N6502::opcode_4e,  6},   // LSR ABS
    { 0, 2 },

    { &N6502::opcode_50,  2},   // BVC
    { &N6502::opcode_51,  5},   // EOR (IND), Y
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_55,  4},   // EOR ZP, X
    { &N6502::opcode_56,  6},   // LSR ZP, X
    { 0, 2 },
    { &N6502::opcode_58,  2},   // CLI
    { &N6502::opcode_59,  4},   // EOR ABS, Y
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_5d,  4},   // EOR ABS, X
    { &N6502::opcode_5e,  7},   // LSR ABS, X
    { 0, 2 },

    { &N6502::opcode_60,  6},   // RTS
    { &N6502::opcode_61,  6},   // ADC (IND, X)
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_65,  3},   // ADC ZP
    { &N6502::opcode_66,  5},   // ROR ZP
    { 0, 2 },
    { &N6502::opcode_68,  4},   // PLA
    { &N6502::opcode_69,  2},   // ADC IMM
    { &N6502::opcode_6a,  2},   // ROR A
    { 0, 2 },
    { &N6502::opcode_6c,  6},   // JMP (ABS)
    { &N6502::opcode_6d,  4},   // ADC ABS
    { &N6502::opcode_6e,  6},   // ROR ABS
    { 0, 2 },

    { &N6502::opcode_70,  2},   // BVS
    { &N6502::opcode_71,  5},   // ADC (IND), Y
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_75,  4},   // ADC ZP, X
    { &N6502::opcode_76,  6},   // ROR ZP, X
    { 0, 2 },
    { &N6502::opcode_78,  2},   // SEI
    { &N6502::opcode_79,  4},   // ADC ABS, Y
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_7d,  4},   // ADC ABS, X
    { &N6502::opcode_7e,  7},   // ROR ABS, X
    { 0, 2 },

    { 0, 2 },
    { &N6502::opcode_81,  6},   // STA (IND, X)
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_84,  3},   // STY ZP
    { &N6502::opcode_85,  3},   // STA ZP
    { &N6502::opcode_86,  3},   // STX ZP
    { 0, 2 },
    { &N6502::opcode_88,  2},   // DEY
    { 0, 2 },
    { &N6502::opcode_8a,  2},   // TXA
    { 0, 2 },
    { &N6502::opcode_8c,  4},   // STY ABS
    { &N6502::opcode_8d,  4},   // STA ABS
    { &N6502::opcode_8e,  4},   // STX ABS
    { 0, 2 },

    { &N6502::opcode_90,  2},   // BCC
    { &N6502::opcode_91,  6},   // STA (IND), Y
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_94,  4},   // STY ZP, X
    { &N6502::opcode_95,  4},   // STA ZP, X
    { &N6502::opcode_96,  4},   // STX ZP, Y
    { 0, 2 },
    { &N6502::opcode_98,  2},   // TYA
    { &N6502::opcode_99,  5},   // STA ABS, Y
    { &N6502::opcode_9a,  2},   // TXS
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_9d,  5},   // STA ABS, X
    { 0, 2 },
    { 0, 2 },

    { &N6502::opcode_a0,  2},   // LDY IMM
    { &N6502::opcode_a1,  6},   // LDA (IND, X)
    { &N6502::opcode_a2,  2},   // LDX IMM
    { 0, 2 },
    { &N6502::opcode_a4,  3},   // LDY ZP
    { &N6502::opcode_a5,  3},   // LDA ZP
    { &N6502::opcode_a6,  3},   // LDX ZP
    { 0, 2 },
    { &N6502::opcode_a8,  2},   // TAY
    { &N6502::opcode_a9,  2},   // LDA IMM
    { &N6502::opcode_aa,  2},   // TAX
    { 0, 2 },
    { &N6502::opcode_ac,  4},   // LDY ABS
    { &N6502::opcode_ad,  4},   // LDA ABS
    { &N6502::opcode_ae,  4},   // LDX ABS
    { 0, 2 },

    { &N6502::opcode_b0,  2},   // BCS
    { &N6502::opcode_b1,  5},   // LDA (IND), Y
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_b4,  4},   // LDY ZP, X
    { &N6502::opcode_b5,  4},   // LDA ZP, X
    { &N6502::opcode_b6,  4},   // LDX ZP, Y
    { 0, 2 },
    { &N6502::opcode_b8,  2},   // CLV
    { &N6502::opcode_b9,  4},   // LDA ABS, Y
    { &N6502::opcode_ba,  2},   // TSX
    { 0, 2 },
    { &N6502::opcode_bc,  4},   // LDY ABS, X
    { &N6502::opcode_bd,  4},   // LDA ABS, X
    { &N6502::opcode_be,  4},   // LDX ABS, Y
    { 0, 2 },

    { &N6502::opcode_c0,  2},   // CPY IMM
    { &N6502::opcode_c1,  6},   // CMP (IND, X)
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_c4,  3},   // CPY ZP
    { &N6502::opcode_c5,  3},   // CMP ZP
    { &N6502::opcode_c6,  5},   // DEC ZP
    { 0, 2 },
    { &N6502::opcode_c8,  2},   // INY
    { &N6502::opcode_c9,  2},   // CMP IMM
    { &N6502::opcode_ca,  2},   // DEX
    { 0, 2 },
    { &N6502::opcode_cc,  4},   // CPY ABS
    { &N6502::opcode_cd,  4},   // CMP ABS
    { &N6502::opcode_ce,  6},   // DEC ABS
    { 0, 2 },

    { &N6502::opcode_d0,  2},   // BNE
    { &N6502::opcode_d1,  5},   // CMP (IND), Y
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_d5,  4},   // CMP ZP, X
    { &N6502::opcode_d6,  6},   // DEC ZP, X
    { 0, 2 },
    { &N6502::opcode_d8,  2},   // CLD
    { &N6502::opcode_d9,  4},   // CMP ABS, Y
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_dd,  4},   // CMP ABS, X
    { &N6502::opcode_de,  7},   // DEC ABS, X
    { 0, 2 },

    { &N6502::opcode_e0,  2},   // CPX IMM
    { &N6502::opcode_e1,  6},   // SBC (IND, X)
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_e4,  3},   // CPX ZP
    { &N6502::opcode_e5,  3},   // SBC ZP
    { &N6502::opcode_e6,  5},   // INC ZP
    { 0, 2 },
    { &N6502::opcode_e8,  2},   // INX
    { &N6502::opcode_e9,  2},   // SBC IMM
    { &N6502::opcode_ea,  2},   // NOP
    { 0, 2 },
    { &N6502::opcode_ec,  4},   // CPX ABS
    { &N6502::opcode_ed,  4},   // SBC ABS
    { &N6502::opcode_ee,  6},   // INC ABS
    { 0, 2 },

    { &N6502::opcode_f0,  2},   // BEQ
    { &N6502::opcode_f1,  5},   // SBC (IND), Y
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_f5,  4},   // SBC ZP, X
    { &N6502::opcode_f6,  6},   // INC ZP, X
    { 0, 2 },
    { &N6502::opcode_f8,  2},   // SED
    { &N6502::opcode_f9,  4},   // SBC ABS, Y
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { &N6502::opcode_fd,  4},   // SBC ABS, X
    { &N6502::opcode_fe,  7},   // INC ABS, X
    { 0, 2 },
};

void N6502::opcode_00()
{
    pushWord( PC+1 );
    pushByte( F | Break );

    F |= IrqDisabled;
    F &= ~DecimalMode; // Not reset in older versions of the CPU

    PC = env_.readWord( 0xFFFE ); // BRK
}

void N6502::opcode_01()
{
    oraByte( env_.readByte( getAddrIndX() ) ); // ORA (IND, X)
}

void N6502::opcode_05()
{
    oraByte( env_.readByte( nextByte() ) ); // ORA ZP
}

void N6502::opcode_06()
{
    unsigned addr = nextByte();
    unsigned char b = aslByte( env_.readByte( addr ) ); // ASL ZP
    env_.writeByte( addr, b );
}

void N6502::opcode_08()
{
    pushByte( F ); // PHP
}

void N6502::opcode_09()
{
    A |= nextByte(); // ORA IMM
    setNZ( A );
}

void N6502::opcode_0a()
{
    A = aslByte( A ); // ASL A
}

void N6502::opcode_0d()
{
    oraByte( env_.readByte( getAddr(0) ) ); // ORA ABS
}

void N6502::opcode_0e()
{
    unsigned addr = getAddr(0);
    unsigned char b = aslByte( env_.readByte( addr ) ); // ASL ABS
    env_.writeByte( addr, b );
}

void N6502::opcode_10()
{
    branchRelative( (F & Sign) == 0 ); // BPL
}

void N6502::opcode_11()
{
    oraByte( env_.readByte( getAddrIndY() ) ); // ORA (IND), Y
}

void N6502::opcode_15()
{
    oraByte( env_.readByte( 0xFF & (nextByte() + X) ) ); // ORA ZP, X
}

void N6502::opcode_16()
{
    unsigned addr = 0xFF & (nextByte() + X);
    unsigned char b = aslByte( env_.readByte( addr ) ); // ASL ZP, X
    env_.writeByte( addr, b );
}

void N6502::opcode_18()
{
    F &= ~Carry; // CLC
}

void N6502::opcode_19()
{
    oraByte( env_.readByte( getAddr(Y) ) ); // ORA ABS, Y
}

void N6502::opcode_1d()
{
    oraByte( env_.readByte( getAddr(X) ) ); // ORA ABS, X
}

void N6502::opcode_1e()
{
    unsigned addr = getAddr(X);
    unsigned char b = aslByte( env_.readByte( addr ) ); // ASL ABS, X
    env_.writeByte( addr, b );
}

void N6502::opcode_20()
{
    pushWord( PC+1 );
    PC = nextWord(); // JSR ABS
}

void N6502::opcode_21()
{
    andByte( env_.readByte( getAddrIndX() ) ); // AND (IND, X)
}

void N6502::opcode_24()
{
    unsigned char b = env_.readByte( nextByte() );

    F &= ~(Zero | Overflow | Sign);
    F |= b & (Overflow | Sign);
    if( (A & b) == 0 ) F |= Zero; // BIT ZP
}

void N6502::opcode_25()
{
    andByte( env_.readByte( nextByte() ) ); // AND ZP
}

void N6502::opcode_26()
{
    unsigned addr = nextByte();
    unsigned char b = rolByte( env_.readByte( addr ) ); // ROL ZP
    env_.writeByte( addr, b );
}

void N6502::opcode_28()
{
    F = popByte(); // PLP
    F |= Bit5;
    F |= Break;
}

void N6502::opcode_29()
{
    andByte( nextByte() ); // AND IMM
}

void N6502::opcode_2a()
{
    A = rolByte( A ); // ROL A
}

void N6502::opcode_2c()
{
    unsigned char b = env_.readByte( getAddr(0) );

    F &= ~(Zero | Overflow | Sign);
    F |= b & (Overflow | Sign);
    if( (A & b) == 0 ) F |= Zero; // BIT ABS
}

void N6502::opcode_2d()
{
    andByte( env_.readByte( getAddr(0) ) ); // AND ABS
}

void N6502::opcode_2e()
{
    unsigned addr = getAddr(0);
    unsigned char b = rolByte( env_.readByte( addr ) ); // ROL ABS
    env_.writeByte( addr, b );
}

void N6502::opcode_30()
{
    branchRelative( (F & Sign) != 0 ); // BMI
}

void N6502::opcode_31()
{
    andByte( env_.readByte( getAddrIndY() ) ); // AND (IND), Y
}

void N6502::opcode_35()
{
    andByte( env_.readByte( 0xFF & (nextByte() + X) ) ); // AND ZP, X
}

void N6502::opcode_36()
{
    unsigned addr = 0xFF & (nextByte() + X);
    unsigned char b = rolByte( env_.readByte( addr ) ); // ROL ZP, X
    env_.writeByte( addr, b );
}

void N6502::opcode_38()
{
    F |= Carry; // SEC
}

void N6502::opcode_39()
{
    andByte( env_.readByte( getAddr(Y) ) ); // AND ABS, Y
}

void N6502::opcode_3d()
{
    andByte( env_.readByte( getAddr(X) ) ); // AND ABS, X
}

void N6502::opcode_3e()
{
    unsigned addr = getAddr(X);
    unsigned char b = rolByte( env_.readByte( addr ) ); // ROL ABS, X
    env_.writeByte( addr, b );
}

void N6502::opcode_40()
{
    F = popByte() | Bit5;
    PC = popWord(); // RTI
}

void N6502::opcode_41()
{
    eorByte( env_.readByte( getAddrIndX() ) ); // EOR (IND, X
}

void N6502::opcode_45()
{
    eorByte( env_.readByte( nextByte() ) ); // EOR ZP
}

void N6502::opcode_46()
{
    unsigned addr = nextByte();
    unsigned char b = lsrByte( env_.readByte( addr ) ); // LSR ZP
    env_.writeByte( addr, b );
}

void N6502::opcode_48()
{
    pushByte( A ); // PHA
}

void N6502::opcode_49()
{
    eorByte( nextByte() ); // EOR IMM
}

void N6502::opcode_4a()
{
    A = lsrByte( A ); // LSR A
}

void N6502::opcode_4c()
{
    PC = nextWord(); // JMP ABS
}

void N6502::opcode_4d()
{
    eorByte( env_.readByte( getAddr(0) ) ); // EOR ABS
}

void N6502::opcode_4e()
{
    unsigned addr = getAddr(0);
    unsigned char b = lsrByte( env_.readByte( addr ) ); // LSR ABS
    env_.writeByte( addr, b );
}

void N6502::opcode_50()
{
    branchRelative( (F & Overflow) == 0 ); // BVC
}

void N6502::opcode_51()
{
    eorByte( env_.readByte( getAddrIndY() ) ); // EOR (IND), Y
}

void N6502::opcode_55()
{
    eorByte( env_.readByte( 0xFF & (nextByte() + X) ) ); // EOR ZP, X
}

void N6502::opcode_56()
{
    unsigned addr = 0xFF & (nextByte() + X);
    unsigned char b = lsrByte( env_.readByte( addr ) ); // LSR ZP, X
    env_.writeByte( addr, b );
}

void N6502::opcode_58()
{
    F &= ~IrqDisabled; // CLI
}

void N6502::opcode_59()
{
    eorByte( env_.readByte( getAddr(Y) ) ); // EOR ABS, Y
}

void N6502::opcode_5d()
{
    eorByte( env_.readByte( getAddr(X) ) ); // EOR ABS, X
}

void N6502::opcode_5e()
{
    unsigned addr = getAddr(X);
    unsigned char b = lsrByte( env_.readByte( addr ) ); // LSR ABS, X
    env_.writeByte( addr, b );
}

void N6502::opcode_60()
{
    PC = popWord() + 1; // RTS
}

void N6502::opcode_61()
{
    adcByte( env_.readByte( getAddrIndX() ) ); // ADC (IND, X)
}

void N6502::opcode_65()
{
    adcByte( env_.readByte( nextByte() ) ); // ADC ZP
}

void N6502::opcode_66()
{
    unsigned addr = nextByte();
    unsigned char b = rorByte( env_.readByte( addr ) ); // ROR ZP
    env_.writeByte( addr, b );
}

void N6502::opcode_68()
{
    A = popByte(); // PLA
    setNZ( A );
}

void N6502::opcode_69()
{
    adcByte( nextByte() ); // ADC IMM
}

void N6502::opcode_6a()
{
    A = rorByte( A ); // ROR A
}

void N6502::opcode_6c()
{
    PC = env_.readWord( nextWord() ); // JMP (ABS)
}

void N6502::opcode_6d()
{
    adcByte( env_.readByte( getAddr(0) ) ); // ADC ABS
}

void N6502::opcode_6e()
{
    unsigned addr = getAddr(0);
    unsigned char b = rorByte( env_.readByte( addr ) ); // ROR ABS
    env_.writeByte( addr, b );
}

void N6502::opcode_70()
{
    branchRelative( (F & Overflow) != 0 ); // BVS
}

void N6502::opcode_71()
{
    adcByte( env_.readByte( getAddrIndY() ) ); // ADC (IND), Y
}

void N6502::opcode_75()
{
    adcByte( env_.readByte( 0xFF & (nextByte() + X) ) ); // ADC ZP, X
}

void N6502::opcode_76()
{
    unsigned addr = 0xFF & (nextByte() + X);
    unsigned char b = rorByte( env_.readByte( addr ) ); // ROR ZP, X
    env_.writeByte( addr, b );
}

void N6502::opcode_78()
{
    F |= IrqDisabled;
}

void N6502::opcode_79()
{
    adcByte( env_.readByte( getAddr(Y) ) ); // ADC ABS, Y
}

void N6502::opcode_7d()
{
    adcByte( env_.readByte( getAddr(X) ) ); // ADC ABS, X
}

void N6502::opcode_7e()
{
    unsigned addr = getAddr(X);
    unsigned char b = rorByte( env_.readByte( addr ) ); // ROR ABS, X
    env_.writeByte( addr, b );
}

void N6502::opcode_81()
{
    env_.writeByte( getAddrIndX(), A ); // STA (IND, X )
}

void N6502::opcode_84()
{
    env_.writeByte( nextByte(), Y ); // STY ZP
}

void N6502::opcode_85()
{
    env_.writeByte( nextByte(), A ); // STA ZP
}

void N6502::opcode_86()
{
    env_.writeByte( nextByte(), X ); // STX ZP
}

void N6502::opcode_88()
{
    Y = setNZ(Y-1); // DEY
}

void N6502::opcode_8a()
{
    A = setNZ(X); // TXA
}

void N6502::opcode_8c()
{
    env_.writeByte( getAddr(0), Y ); // STY ABS
}

void N6502::opcode_8d()
{
    env_.writeByte( getAddr(0), A ); // STA ABS
}

void N6502::opcode_8e()
{
    env_.writeByte( getAddr(0), X ); // STX ABS
}

void N6502::opcode_90()
{
    branchRelative( (F & Carry) == 0 ); // BCC
}

void N6502::opcode_91()
{
    env_.writeByte( getAddrIndY(), A ); // STA (IND), Y
}

void N6502::opcode_94()
{
    env_.writeByte( 0xFF & (nextByte() + X), Y ); // STY ZP, X
}

void N6502::opcode_95()
{
    env_.writeByte( 0xFF & (nextByte() + X), A ); // STA ZP, X
}

void N6502::opcode_96()
{
    env_.writeByte( 0xFF & (nextByte() + Y), X ); // STX ZP, Y
}

void N6502::opcode_98()
{
    A = setNZ(Y); // TYA
}

void N6502::opcode_99()
{
    env_.writeByte( getAddr(Y), A ); // STA ABS, Y
}

void N6502::opcode_9a()
{
    S = X; // TXS
}

void N6502::opcode_9d()
{
    env_.writeByte( getAddr(X), A ); // STA ABS, X
}

void N6502::opcode_a0()
{
    Y = setNZ( nextByte() ); // LDY IMM
}

void N6502::opcode_a1()
{
    A = env_.readByte( getAddrIndX() ); // LDA (IND, X )
    setNZ( A );
}

void N6502::opcode_a2()
{
    X = setNZ( nextByte() ); // LDX IMM
}

void N6502::opcode_a4()
{
    Y = env_.readByte( nextByte() ); // LDY ZP
    setNZ( Y );
}

void N6502::opcode_a5()
{
    A = env_.readByte( nextByte() ); // LDA ZP
    setNZ( A );
}

void N6502::opcode_a6()
{
    X = env_.readByte( nextByte() ); // LDX ZP
    setNZ( X );
}

void N6502::opcode_a8()
{
    Y = setNZ(A); // TAY
}

void N6502::opcode_a9()
{
    A = setNZ( nextByte() ); // LDA IMM
}

void N6502::opcode_aa()
{
    X = setNZ(A); // TAX
}

void N6502::opcode_ac()
{
    Y = env_.readByte( getAddr(0) ); // LDY ABS
    setNZ( Y );
}

void N6502::opcode_ad()
{
    A = env_.readByte( getAddr(0) ); // LDA ABS
    setNZ( A );
}

void N6502::opcode_ae()
{
    X = env_.readByte( getAddr(0) ); // LDX ABS
    setNZ( X );
}

void N6502::opcode_b0()
{
    branchRelative( (F & Carry) != 0 ); // BCS
}

void N6502::opcode_b1()
{
    A = env_.readByte( getAddrIndY() ); // LDA (IND), Y
    setNZ( A );
}

void N6502::opcode_b4()
{
    Y = env_.readByte( 0xFF & (nextByte() + X) ); // LDY ZP, X
    setNZ( Y );
}

void N6502::opcode_b5()
{
    A = env_.readByte( 0xFF & (nextByte() + X) ); // LDA ZP, X
    setNZ( A );
}

void N6502::opcode_b6()
{
    X = env_.readByte( 0xFF & (nextByte() + Y) ); // LDX ZP, Y
    setNZ( X );
}

void N6502::opcode_b8()
{
    F &= ~Overflow; // CLV
}

void N6502::opcode_b9()
{
    A = env_.readByte( getAddr(Y) ); // LDA ABS, Y
    setNZ( A );
}

void N6502::opcode_ba()
{
    X = S; // TSX
    setNZ( X );
}

void N6502::opcode_bc()
{
    Y = env_.readByte( getAddr(X) ); // LDY ABS, X
    setNZ( Y );
}

void N6502::opcode_bd()
{
    A = env_.readByte( getAddr(X) ); // LDA ABS, X
    setNZ( A );
}

void N6502::opcode_be()
{
    X = env_.readByte( getAddr(Y) ); // LDX ABS, Y
    setNZ( X );
}

void N6502::opcode_c0()
{
    cmpByte( Y, nextByte() ); // CPY IMM
}

void N6502::opcode_c1()
{
    cmpByte( A, env_.readByte( getAddrIndX() ) ); // CMP (IND, X)
}

void N6502::opcode_c4()
{
    cmpByte( Y, env_.readByte( nextByte() ) ); // CPY ZP
}

void N6502::opcode_c5()
{
    cmpByte( A, env_.readByte( nextByte() ) ); // CMP ZP
}

void N6502::opcode_c6()
{
    unsigned addr = nextByte();
    unsigned char b = env_.readByte( addr );
    env_.writeByte( addr, setNZ(b-1) ); // DEC ZP
}

void N6502::opcode_c8()
{
    Y = setNZ(Y+1); // INY
}

void N6502::opcode_c9()
{
    cmpByte( A, nextByte() ); // CMP IMM
}

void N6502::opcode_ca()
{
    X = setNZ(X-1); // DEX
}

void N6502::opcode_cc()
{
    cmpByte( Y, env_.readByte( getAddr(0) ) ); // CPY ABS
}

void N6502::opcode_cd()
{
    cmpByte( A, env_.readByte( getAddr(0) ) ); // CMP ABS
}

void N6502::opcode_ce()
{
    unsigned addr = getAddr(0);
    unsigned char b = env_.readByte( addr );
    env_.writeByte( addr, setNZ(b-1) ); // DEC ABS
}

void N6502::opcode_d0()
{
    branchRelative( (F & Zero) == 0 ); // BNE
}

void N6502::opcode_d1()
{
    cmpByte( A, env_.readByte( getAddrIndY() ) ); // CMP (IND), Y
}

void N6502::opcode_d5()
{
    cmpByte( A, env_.readByte( 0xFF & (nextByte() + X) ) ); // CMP ZP, X
}

void N6502::opcode_d6()
{
    unsigned addr = 0xFF & (nextByte() + X);
    unsigned char b = env_.readByte( addr );
    env_.writeByte( addr, setNZ(b-1) ); // DEC ZP, X
}

void N6502::opcode_d8()
{
    F &= ~DecimalMode; // CLD
}

void N6502::opcode_d9()
{
    cmpByte( A, env_.readByte( getAddr(Y) ) ); // CMP ABS, Y
}

void N6502::opcode_dd()
{
    cmpByte( A, env_.readByte( getAddr(X) ) ); // CMP ABS, X
}

void N6502::opcode_de()
{
    unsigned addr = getAddr(X);
    unsigned char b = env_.readByte( addr );
    env_.writeByte( addr, setNZ(b-1) ); // DEC ABS, X
}

void N6502::opcode_e0()
{
    cmpByte( X, nextByte() ); // CPX IMM
}

void N6502::opcode_e1()
{
    sbcByte( env_.readByte( getAddrIndX() ) ); // SBC (IND, X)
}

void N6502::opcode_e4()
{
    cmpByte( X, env_.readByte( nextByte() ) ); // CPX ZP
}

void N6502::opcode_e5()
{
    sbcByte( env_.readByte( nextByte() ) ); // SBC ZP
}

void N6502::opcode_e6()
{
    unsigned addr = nextByte();
    unsigned char b = env_.readByte( addr );
    env_.writeByte( addr, setNZ(b+1) ); // INC ZP
}

void N6502::opcode_e8()
{
    X = setNZ(X+1); // INX
}

void N6502::opcode_e9()
{
    sbcByte( nextByte() ); // SBC IMM
}

void N6502::opcode_ea()
{
    // NOP
}

void N6502::opcode_ec()
{
    cmpByte( X, env_.readByte( getAddr(0) ) ); // CPX ABS
}

void N6502::opcode_ed()
{
    sbcByte( env_.readByte( getAddr(0) ) ); // SBC ABS
}

void N6502::opcode_ee()
{
    unsigned addr = getAddr(0);
    unsigned char b = env_.readByte( addr );
    env_.writeByte( addr, setNZ(b+1) ); // INC ABS
}

void N6502::opcode_f0()
{
    branchRelative( (F & Zero) != 0 ); // BEQ
}

void N6502::opcode_f1()
{
    sbcByte( env_.readByte( getAddrIndY() ) ); // SBC (IND), Y
}

void N6502::opcode_f5()
{
    sbcByte( env_.readByte( 0xFF & (nextByte() + X) ) ); // SBC ZP, X
}

void N6502::opcode_f6()
{
    unsigned addr = 0xFF & (nextByte() + X);
    unsigned char b = env_.readByte( addr );
    env_.writeByte( addr, setNZ(b+1) ); // INC ZP, X
}

void N6502::opcode_f8()
{
    F |= DecimalMode; // SED
}

void N6502::opcode_f9()
{
    sbcByte( env_.readByte( getAddr(Y) ) ); // SBC ABS, Y
}

void N6502::opcode_fd()
{
    sbcByte( env_.readByte( getAddr(X) ) ); // SBC ABS, X
}

void N6502::opcode_fe()
{
    unsigned addr = getAddr(X);
    unsigned char b = env_.readByte( addr );
    env_.writeByte( addr, setNZ(b+1) ); // INC ABS, X
}
