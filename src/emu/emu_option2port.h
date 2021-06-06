/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#ifndef EMU_OPTION2PORT_H_
#define EMU_OPTION2PORT_H_

#include "emu_input.h"
#include "emu_ui.h"

class TOptionToPortHandler
{
public:
    TOptionToPortHandler() {
    }

    ~TOptionToPortHandler();

    void add( unsigned id, unsigned param, unsigned char * port, unsigned char and_mask, unsigned char or_mask );

    void add( unsigned char * port, TUserInterface * ui, unsigned id );

    void add( unsigned char * port, TUserInterface * ui, unsigned id1, unsigned id2 );

    void add( unsigned char * port, TUserInterface * ui, unsigned id1, unsigned id2, unsigned id3 );

    void add( unsigned char * port, TUserInterface * ui, unsigned id1, unsigned id2, unsigned id3, unsigned id4 );

    void add( unsigned char * port, TUserInterface * ui, unsigned id1, unsigned id2, unsigned id3, unsigned id4, unsigned id5 );

    void resynch( TUserInterface * ui );

    void clear();

    bool handleInputEvent( unsigned device, unsigned param );

private:
    unsigned xlatDevice( unsigned device, unsigned param ) {
        return (TInput::getDeviceId(device) << 8) | (param & 0xFF);
    }

    TList options_;
};

#endif // EMU_OPTION2PORT_H_
