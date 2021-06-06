/*
    N6502 emulator

    Copyright (c) 2011 Alessandro Scotti
*/
#include "n6502.h"

const bool HaveBcdFlags = false;

void N6502::branchRelative( bool cond )
{
    int ofs = env_.readByte( PC++ );

    if( cond ) {
        unsigned oldpage = PC >> 8;

        if( ofs >= 0x80 ) ofs = ofs - 0x100;

        PC += ofs;

        unsigned newpage = PC >> 8;

        cycles_++;

        if( oldpage != newpage ) cycles_++;
    }
}

void N6502::adcByte( unsigned char op )
{
    if( F & DecimalMode ) {
        cycles_++;

        unsigned char x = A + op + ((F & Carry) ? 1 : 0);

        unsigned char bcd_lo = (A & 0x0F) + (op & 0x0F) + ((F & Carry) ? 1 : 0);
        if( bcd_lo > 9 ) bcd_lo += 6;
        unsigned char bcd_hi = (A >> 4) + (op >>4) + ((bcd_lo > 0x0F) ? 1 : 0);

        F &= ~Carry;

        if( bcd_hi > 0x0F ) F |= Carry;

        if( HaveBcdFlags ) {
            F &= ~(Zero | Overflow | Sign);
            if( x == 0 ) F |= Zero;
            if( bcd_hi & 0x08 ) F |= Sign;
            if( ~(A ^ op) & (x ^ op) & 0x80 ) F |= Overflow; // See I8080 code for explanation of this
        }

        A = (bcd_hi << 4) | (bcd_lo & 0x0F);
    }
    else {
        unsigned x = A + op + ((F & Carry) ? 1 : 0);

        F &= ~(Carry | Zero | Overflow | Sign);

        if( x >= 0x100 ) F |= Carry;

        if( ~(A ^ op) & (x ^ op) & 0x80 ) F |= Overflow; // See I8080 code for explanation of this

        A = (unsigned char) x;

        setNZ(A);
    }
}

void N6502::sbcByte( unsigned char op )
{
    if( F & DecimalMode ) {
        cycles_++;

        unsigned char x = A - op - ((F & Carry) ? 0 : 1);

        unsigned char bcd_lo = (A & 0x0F) - (op & 0x0F) - ((F & Carry) ? 0 : 1);
        if( bcd_lo & 0x10 ) bcd_lo -= 6;
        unsigned char bcd_hi = (A >> 4) - (op >>4) - ((bcd_lo & 0x10) ? 1 : 0);
        if( bcd_hi & 0x10 ) bcd_hi -= 6;

        F &= ~Carry;

        if( bcd_hi < 0x10 ) F |= Carry;

        if( HaveBcdFlags ) {
            F &= ~(Zero | Overflow | Sign);
            if( x == 0 ) F |= Zero;
            if( bcd_hi & 0x08 ) F |= Sign;
            if( (A ^ op) & (A ^ x) & 0x80 ) F |= Overflow; // See I8080 code for explanation of this
        }

        A = (bcd_hi << 4) | (bcd_lo & 0x0F);
    }
    else {
        unsigned x = A - op - ((F & Carry) ? 0 : 1);

        F &= ~(Carry | Zero | Overflow | Sign);

        if( (x & ~0xFF) == 0 ) F |= Carry;

        if( (A ^ op) & (A ^ x) & 0x80 ) F |= Overflow; // See I8080 code for explanation of this

        A = (unsigned char) x;

        setNZ(A);
    }
}

unsigned char N6502::setNZ( unsigned char b )
{
    F &= ~(Zero | Sign);

    if( b & 0x80 ) F |= Sign;
    if( b == 0 ) F |= Zero;

    return b;
}

unsigned char N6502::nextByte()
{
    return env_.readByte( PC++ );
}

unsigned N6502::nextWord()
{
    unsigned x = env_.readWord( PC );
    PC = (PC + 2) & 0xFFFF;
    return x;
}

void N6502::pushByte( unsigned char b )
{
    env_.writeByte( 0x100 | S, b );
    S--;
}

void N6502::pushWord( unsigned w )
{
    pushByte( w >> 8 );
    pushByte( w & 0xFF );
}

unsigned char N6502::popByte()
{
    S++;
    return env_.readByte( 0x100 | S );
}

unsigned N6502::popWord()
{
    unsigned lo = popByte();
    unsigned hi = popByte();

    return (hi << 8) | lo;
}

void N6502::andByte( unsigned char b )
{
    A &= b;
    setNZ(A);
}

void N6502::cmpByte( unsigned char op1, unsigned char op2 )
{
    unsigned x = op1 - op2;

    F &= ~(Carry | Zero | Sign);
    if( (x & ~0xFF) == 0 ) F |= Carry;
    setNZ( x & 0xFF );
}

void N6502::eorByte( unsigned char b )
{
    A ^= b;
    setNZ(A);
}

unsigned char N6502::aslByte( unsigned char b )
{
    F &= ~Carry;
    return rolByte( b );
}

unsigned char N6502::lsrByte( unsigned char b )
{
    F &= ~Carry;
    return rorByte( b );
}

void N6502::oraByte( unsigned char b )
{
    A |= b;
    setNZ(A);
}

unsigned char N6502::rolByte( unsigned char b )
{
    unsigned char r = (b << 1) | ((F & Carry) ? 0x01 : 0x00);
    F &= ~Carry;
    if( b & 0x80 ) F |= Carry;
    return setNZ( r );
}

unsigned char N6502::rorByte( unsigned char b )
{
    unsigned char r = (b >> 1) | ((F & Carry) ? 0x80 : 0x00);
    F &= ~Carry;
    if( b & 0x01 ) F |= Carry;
    return setNZ( r );
}

unsigned N6502::getAddr( unsigned ofs )
{
    unsigned base = nextWord();
    unsigned addr = (base + ofs) & 0xFFFF;
    if( (base & 0xFF00) != (addr & 0xFF00) ) cycles_++;
    return addr;
}

unsigned N6502::getAddrIndX()
{
    unsigned addr = nextByte();
    addr = (addr + X) & 0xFF;
    addr = env_.readWord( addr );
    return addr;
}

unsigned N6502::getAddrIndY()
{
    unsigned base = nextByte();
    base = env_.readWord( base );
    unsigned addr = (base + Y) & 0xFFFF;
    if( (base & 0xFF00) != (addr & 0xFF00) ) cycles_++;
    return addr;
}
