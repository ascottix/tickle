/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#include "emu_iostream.h"

int TInputStream::read()
{
    char b;

    return read( &b, 1 ) == 1 ? (int)b : -1;
}

unsigned TInputStream::skip( unsigned n )
{
    unsigned t = n;

    while( n > 0 ) {
        if( read() < 0 ) {
            break;
        }
        n--;
    }

    return t - n;
}

int TOutputStream::write( int c )
{
    char b = (char) c;

    return write( &b, 1 ) == 1 ? c : -1;
}
