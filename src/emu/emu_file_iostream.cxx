/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#include "emu_file_iostream.h"

TFileInputStream::~TFileInputStream() 
{
    fclose( f_ );
}

unsigned TFileInputStream::read( void * buf, unsigned len ) 
{
    return (unsigned) fread( buf, 1, (size_t) len, f_ );
}

unsigned TFileInputStream::skip( unsigned n )
{
    unsigned result = 0;

    long offset = (long) n;

    if( offset > 0 ) {
        long curpos = ftell( f_ );
        result = (fseek( f_, offset, SEEK_CUR ) == 0) ? n : (unsigned) (ftell(f_)-curpos);
    }

    return result;
}

TFileOutputStream::~TFileOutputStream() 
{
    flush();
    fclose( f_ );
}

unsigned TFileOutputStream::write( const char * buf, unsigned len )
{
    return (unsigned) fwrite( buf, 1, (size_t)len, f_ );
}

void TFileOutputStream::flush()
{
    fflush( f_ );
}
