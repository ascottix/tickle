/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef EMU_RESOURCE_HANDLER_H_
#define EMU_RESOURCE_HANDLER_H_

#include "emu_info.h"

struct TResourceHandlerReplaceInfo
{
    int id;
    const char * name;
    unsigned crc;
};

class TResourceHandler
{
public:
    TResourceHandler() {
    }

    ~TResourceHandler();

    void add( int id, const char * name, unsigned len, TResourceFileType type, unsigned char * buf, unsigned crc = 0 );

    void replace( int id, const char * name, unsigned crc, unsigned char * buf = 0 );

    void replace( TResourceHandlerReplaceInfo * info, int count = -1 );

    void remove( int id );

    void clear();

    void addToMachineDriverInfo( TMachineDriverInfo * di );

    void assignToMachineDriverInfo( TMachineDriverInfo * di ) {
        di->clearResources();
        addToMachineDriverInfo( di );
    }

    int handle( int id, const unsigned char * buf, unsigned len );

private:
    TList items_;
};

#endif // EMU_RESOURCE_HANDLER_H_
