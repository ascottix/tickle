/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#include "emu_input_manager.h"

struct TEmuInputManagerItem
{
    unsigned key;
    unsigned id;
    unsigned data;
};

TEmuInputManager::TEmuInputManager()
{
    count_ = 0;
    capacity_ = 0;
    items_ = 0;
}

TEmuInputManager::~TEmuInputManager()
{
    delete [] items_;

    for( int i=0; i<joysticks_.count(); i++ ) {
        TJoystick * joystick = (TJoystick *) joysticks_.item(i);

        delete joystick;
    }
}

void TEmuInputManager::expand()
{
    if( count_ == capacity_ ) {
        capacity_ += 16;
        TEmuInputManagerItem * items = new TEmuInputManagerItem[ capacity_ ];
        for( int i=0; i<count_; i++ ) {
            items[i] = items_[i];
        }
        delete [] items_;
        items_ = items;
    }
}

void TEmuInputManager::add( unsigned key, unsigned id, unsigned data )
{
    if( key != 0 ) {
        expand();

        // Find the position where this item must be inserted
        int index = 0;
    
        while( (index < count_) && (items_[index].key < key) ) {
            index++;
        }

        // Make place for the new item
        for( int i=count_; i>index; i-- ) {
            items_[i] = items_[i-1];
        }

        // Add the item
        items_[index].key = key;
        items_[index].id = id;
        items_[index].data = data;

        count_++;
    }
}

TJoystick * TEmuInputManager::addJoystick( unsigned id, unsigned kleft, unsigned kright, unsigned kup, unsigned kdown )
{
    int index = -1;

    // Check if we already have this id, add if not found
    for( int i=0; i<joysticks_.count(); i++ ) {
        TJoystick * joystick = (TJoystick *) joysticks_.item(i);

        if( joystick->id() == id ) {
            index = i;
            break;
        }
    }

    if( index < 0 ) {
        TJoystick * joystick = new TJoystick( id );

        index = joysticks_.add( joystick );
    }

    // Add the key mappings
    add( kleft, dcJoystick, (TJoystick::KeyLeft << 2) | index );
    add( kright, dcJoystick, (TJoystick::KeyRight << 2) | index );
    add( kup, dcJoystick, (TJoystick::KeyUp << 2) | index );
    add( kdown, dcJoystick, (TJoystick::KeyDown << 2) | index );

    return (TJoystick *) joysticks_.item(index);
}

void TEmuInputManager::clear()
{
    count_ = 0;
}

bool TEmuInputManager::handle( unsigned key, unsigned param, TMachine * machine )
{
    bool result = false;

    int lo = 0;
    int hi = count_ - 1;

    // Do a binary search for the specified device
    while( lo <= hi ) {
        int i = (lo + hi) / 2;

        if( items_[i].key < key ) {
            lo = i + 1;
        }
        else {
            if( items_[i].key == key ) {
                // Found, dispatch the event
                unsigned id = items_[i].id;
                if( TInput::getDeviceClass( id ) == dcJoystick ) {
                    unsigned j = items_[i].data & 3;
                    unsigned k = items_[i].data >> 2;

                    TJoystick * joystick = (TJoystick *) joysticks_.item(j);

                    joystick->handleArrowKey( k, param );
                }
                else {
                    machine->handleInputEvent( id, items_[i].data | param, 0 );
                }
                result = true;
                break;
            }

            hi = i - 1;
        }
    }

    return result;
}

void TEmuInputManager::notifyJoysticks( TMachine * machine )
{
    for( int i=0; i<joysticks_.count(); i++ ) {
        TJoystick * joystick = (TJoystick *) joysticks_.item(i);

        joystick->notify( machine );
    }
}
