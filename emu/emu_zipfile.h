/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef EMU_ZIPFILE_H_
#define EMU_ZIPFILE_H_

#include <stdio.h>

#include "emu_iostream.h"
#include "emu_list.h"
#include "emu_string.h"

class TZipFile;

/**
    Entry in a ZIP archive.
*/
class TZipEntry
{
public:
    /** Destructor. */
    virtual ~TZipEntry() {
        delete name_;
    }

    const char * name() const {
        return name_;
    }

    unsigned size() const {
        return size_;
    }

    unsigned crc() const {
        return crc_;
    }

    TInputStream * open() const;

private:
    friend class TZipFile;

    TZipEntry( char * name, unsigned size, unsigned csize, unsigned offset, unsigned crc, int method, TZipFile * owner );
    
    unsigned offset() const {
        return offset_;
    }

    unsigned csize() const {
        return csize_;
    }

    int method() const {
        return method_;
    }

    TZipEntry( const TZipEntry & );
    TZipEntry & operator = ( const TZipEntry & );

    char * name_;
    unsigned size_;
    unsigned csize_;
    unsigned crc_;
    unsigned offset_;
    int method_;
    TZipFile * owner_;
};

class TZipFile
{
public:
    /** Destructor. */
    virtual ~TZipFile();

    const TZipEntry * entry( int index ) {
        return (TZipEntry *) entries_[index];
    }

    const TZipEntry * entry( const char * name, bool nameonly = false );

    int count() const {
        return entries_.count();
    }

    static TZipFile * open( const char * name );

private:
    friend class TZipEntry;

    TInputStream * openEntry( const TZipEntry * entry ) const;

    TZipFile( FILE * f );

    FILE * f_;
    TList entries_;
};

#endif // EMU_ZIPFILE_H_
