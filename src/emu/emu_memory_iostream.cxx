/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#include "emu_memory_iostream.h"

TMemoryInputStream::TMemoryInputStream( const unsigned char * data, unsigned size ) 
    : data_(data), size_(size) 
{
    offset_ = 0;
}

unsigned TMemoryInputStream::read( void * buf, unsigned len ) {
    unsigned result = 0;

    if( offset_ < size_ ) {
        unsigned max_len = size_ - offset_;

        if( len > max_len ) {
            len = max_len;
        }

        memcpy( buf, data_+offset_, len );
        offset_ += len;

        result = len;
    }

    return result;
}
