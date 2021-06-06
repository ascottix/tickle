/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#include "emu_option2port.h"

struct TOptionItem {
    unsigned id;
    unsigned index;
    unsigned char * port;
    unsigned char and_mask;
    unsigned char or_mask;
};

TOptionToPortHandler::~TOptionToPortHandler()
{
    clear();
}

void TOptionToPortHandler::clear()
{
    for( int i=0; i<options_.count(); i++ ) {
        TOptionItem * oi = (TOptionItem *) options_.item(i);

        delete oi;
    }

    options_.clear();
}

void TOptionToPortHandler::add( unsigned id, unsigned param, unsigned char * port, unsigned char and_mask, unsigned char or_mask )
{
    TOptionItem * opt = new TOptionItem();

    opt->id = id;
    opt->index = param;
    opt->port = port;
    opt->and_mask = and_mask;
    opt->or_mask = or_mask;

    options_.add( opt );
}

void TOptionToPortHandler::resynch( TUserInterface * ui )
{
    TList items;
    int i;

    // Move items into a temporary list
    for( i=0; i<options_.count(); i++ ) {
        items.add( options_.item(i) );
    }

    options_.clear();

    // Scan the list and re-add good items
    for( i=0; i<items.count(); i++ ) {
        TOptionItem * opt = (TOptionItem *) items.item(i);
        bool itemIsGood = true;

        for( int j=0; j<options_.count(); j++ ) {
            TOptionItem * o = (TOptionItem *) options_.item(j);
            if( o->id == opt->id ) {
                itemIsGood = false;
                break;
            }
        }

        if( itemIsGood && ui->findOption(opt->id) ) {
            add( opt->port, ui, opt->id );
        }

        delete opt;
    }
}
        
bool TOptionToPortHandler::handleInputEvent( unsigned device, unsigned param )
{
    bool result = false;

    if( device & dcOption ) {
        unsigned id = TInput::getDeviceId(device);

        for( int i=0; i<options_.count(); i++ ) {
            TOptionItem * opt = (TOptionItem *) options_.item(i);

            if( (opt->id == id) && (opt->index == param) ) {
                *(opt->port) &= ~opt->and_mask;
                *(opt->port) |= opt->or_mask;
                result = true;
                break;
            }
        }
    }

    return result;
}

void TOptionToPortHandler::add( unsigned char * port, TUserInterface * ui, unsigned id )
{
    for( int i=0; i<ui->groupCount(); i++ ) {
        TUiOptionGroup * group = ui->group( i );

        for( int j=0; j<group->count(); j++ ) {
            TUiOption * option = group->item( j );
            unsigned option_class = TInput::getDeviceClass( option->id() );
            unsigned option_id = TInput::getDeviceId( option->id() );

            if( (option_class & dcOption) && (option_id == id) ) {
                for( int k=0; k<option->count(); k++ ) {
                    add( id, k, port, option->mask(), option->itemValue(k) );
                }
            }
        }
    }
}

void TOptionToPortHandler::add( unsigned char * port, TUserInterface * ui, unsigned id1, unsigned id2 )
{
    add( port, ui, id1 );
    add( port, ui, id2 );
}

void TOptionToPortHandler::add( unsigned char * port, TUserInterface * ui, unsigned id1, unsigned id2, unsigned id3 )
{
    add( port, ui, id1, id2 );
    add( port, ui, id3 );
}

void TOptionToPortHandler::add( unsigned char * port, TUserInterface * ui, unsigned id1, unsigned id2, unsigned id3, unsigned id4 )
{
    add( port, ui, id1, id2, id3 );
    add( port, ui, id4 );
}

void TOptionToPortHandler::add( unsigned char * port, TUserInterface * ui, unsigned id1, unsigned id2, unsigned id3, unsigned id4, unsigned id5 )
{
    add( port, ui, id1, id2, id3, id4 );
    add( port, ui, id5 );
}
