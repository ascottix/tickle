/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#include <assert.h>

#include "emu_event2port.h"

class TEventToPortItem
{
public:
    void set( unsigned device, int type, unsigned char * port, unsigned char and_mask, unsigned char or_mask ) {
        device_ = device;
        type_ = type;
        port_ = port;
        and_mask_ = and_mask;
        or_mask_ = or_mask;
    }

    unsigned device_;
    int type_;
    unsigned char * port_;
    unsigned char and_mask_;
    unsigned char or_mask_;
};

TEventToPortHandler::TEventToPortHandler()
{
    count_ = 0;
    capacity_ = 0;
    items_ = 0;
}

TEventToPortHandler::~TEventToPortHandler()
{
    delete [] items_;
}

void TEventToPortHandler::expand()
{
    if( count_ == capacity_ ) {
        capacity_ += 16;
        TEventToPortItem * items = new TEventToPortItem[ capacity_ ];
        for( int i=0; i<count_; i++ ) {
            items[i] = items_[i];
        }
        delete [] items_;
        items_ = items;
    }
}

void TEventToPortHandler::add( unsigned device, int type, unsigned char * port, unsigned char and_mask, unsigned char or_mask )
{
    // Remove this item if already in the list
    remove( device );

    // Make sure there is enough space for this item
    expand();

    // Find the position where this item must be inserted
    int index = 0;
    
    while( (index < count_) && (items_[index].device_ < device) ) {
        index++;
    }

    // Make place for the new item
    for( int i=count_; i>index; i-- ) {
        items_[i] = items_[i-1];
    }

    // Add the item
    items_[index].set( device, type, port, and_mask, or_mask );

    count_++;
}

bool TEventToPortHandler::remove( unsigned device )
{
    bool result = false;

    for( int i=0; i<count_; i++ ) {
        if( items_[i].device_ == device ) {
            for( int j=i+1; j<count_; j++ ) {
                items_[j-1] = items_[j];
            }
            count_--;
            result = true;
            break;
        }
    }

    return result;
}

bool TEventToPortHandler::handleInputEvent( unsigned device, unsigned param )
{
    bool result = false;

    int lo = 0;
    int hi = count_ - 1;

    // Do a binary search for the specified device
    while( lo <= hi ) {
        int i = (lo + hi) / 2;

        if( items_[i].device_ < device ) {
            lo = i + 1;
        }
        else {
            if( items_[i].device_ == device ) {
                // Found, handle the event
                unsigned char * port = items_[i].port_;
                int type = items_[i].type_;

                assert( port != 0 );

                *port &= ~items_[i].and_mask_;
                if( (param && (type == ptNormal)) || (!param && (type == ptInverted)) ) {
                    // Set the bits
                    *port |= items_[i].or_mask_;
                }

                result = true;
                break;
            }

            hi = i - 1;
        }
    }

    return result;
}
