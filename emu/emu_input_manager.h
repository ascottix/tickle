/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef EMU_INPUT_MANAGER_H_
#define EMU_INPUT_MANAGER_H_

#include "emu_joystick.h"
#include "emu_list.h"
#include "emu_machine.h"

struct TEmuInputManagerItem;

class TEmuInputManager
{
public:
    TEmuInputManager();

    ~TEmuInputManager();

    void add( unsigned key, unsigned id, unsigned data = 0 );

    TJoystick * addJoystick( unsigned id, unsigned kleft, unsigned kright, unsigned kup, unsigned kdown );

    void clear();

    bool handle( unsigned key, unsigned param, TMachine * machine );

    void notifyJoysticks( TMachine * machine );

    TJoystick * joystick( int index ) {
        return (TJoystick *) joysticks_.item(index);
    }

private:
    void expand();

    int count_;
    int capacity_;
    TEmuInputManagerItem * items_;
    TList joysticks_;
};

#endif // EMU_INPUT_MANAGER_H_
