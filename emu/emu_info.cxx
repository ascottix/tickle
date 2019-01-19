/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include "emu_info.h"

TMachineDriverInfo::~TMachineDriverInfo() 
{
    clearResources();
}

void TMachineDriverInfo::clearResources()
{
    for( int i=0; i<resourceFileCount(); i++ ) {
        delete (TResourceFileInfo *) resourceFile( i );
    }

    resource_files_.clear();
}

void TMachineDriverInfo::addResourceFile( int id,  const char * name, unsigned crc, unsigned size, TResourceFileType type ) 
{
    // Delete any existing item with the same identifier
    for( int i=0; i<resourceFileCount(); i++ ) {
        TResourceFileInfo * fi = (TResourceFileInfo *) resource_files_.item(i);

        if( fi->id == id ) {
            resource_files_.remove( i );
            delete fi;
            break;
        }
    }

    // Add the item
    resource_files_.add( new TResourceFileInfo( id, name, crc, size, type ) );
}
