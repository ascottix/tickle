/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef EMU_FILE_IOSTREAM_H_
#define EMU_FILE_IOSTREAM_H_

#include <stdio.h>

#include "emu_iostream.h"

class TFileInputStream : public TInputStream
{
public:
    /** Destructor. */
    virtual ~TFileInputStream();

    virtual unsigned read( void * buf, unsigned len );

    virtual unsigned skip( unsigned n );

    static TFileInputStream * open( const char * name ) {
        FILE * f = fopen( name, "rb" );

        return (f != 0) ? new TFileInputStream(f) : 0;
    }

private:
    TFileInputStream( FILE * f ) : f_(f) {
    }

    TFileInputStream();
    TFileInputStream( const TFileInputStream & );
    TFileInputStream & operator = ( const TFileInputStream & );

    FILE * f_;
};

class TFileOutputStream : public TOutputStream
{
public:
    /** Destructor. */
    virtual ~TFileOutputStream();

    virtual unsigned write( const char * buf, unsigned len );
    
    virtual void flush();

    static TFileOutputStream * open( const char * name, bool append ) {
        FILE * f = fopen( name, append ? "ab" : "wb" );

        return (f != 0) ? new TFileOutputStream(f) : 0;
    }

private:
    TFileOutputStream( FILE * f ) : f_(f) {
    }
    
    TFileOutputStream();
    TFileOutputStream( const TFileInputStream & );
    TFileOutputStream & operator = ( const TFileInputStream & );

    FILE * f_;
};

#endif // EMU_FILE_IOSTREAM_H_
