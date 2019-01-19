/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef EMU_MEMORY_IOSTREAM_H_
#define EMU_MEMORY_IOSTREAM_H_

#include "emu_iostream.h"

#include <string.h>

class TMemoryInputStream : public TInputStream
{
public:
    TMemoryInputStream( const unsigned char * data, unsigned size );

    virtual ~TMemoryInputStream() {
    }

    virtual unsigned read( void * buf, unsigned len );

private:
    const unsigned char * data_;
    unsigned size_;
    unsigned offset_;
};

#endif // EMU_MEMORY_IOSTREAM_H_
