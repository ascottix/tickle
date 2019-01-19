/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef EMU_IOSTREAM_H_
#define EMU_IOSTREAM_H_

class TInputStream
{
public:
    TInputStream() {
    }

    /** Destructor. */
    virtual ~TInputStream() {
    }

    virtual int read();

    virtual unsigned read( void * buf, unsigned len ) = 0;

    virtual unsigned skip( unsigned n );
};

class TOutputStream
{
public:
    TOutputStream() {
    }
    
    /** Destructor. */
    virtual ~TOutputStream() {
    }

    virtual int write( int c );
    
    virtual unsigned write( const char * buf, unsigned len ) = 0;
    
    virtual void flush() {
    }
};

#endif // EMU_IOSTREAM_H_
