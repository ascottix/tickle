/*
    Tickle
    Emulation library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#include <string.h>

#include "emu_machine.h"
#include "emu_option2port.h"

class TUiOptionItem
{
public:
    TUiOptionItem( const char * name, unsigned char value ) : name_(name), value_(value) {
    }

    const char * name_;
    unsigned char value_;
};

TUiOption::TUiOption( unsigned id, const char * name, int def, unsigned char mask ) : 
    name_(name), id_(id), mask_(mask), default_(def), selected_(def)
{
}

TUiOption::~TUiOption()
{
    for( int i=0; i<count(); i++ ) {
        TUiOptionItem * t = (TUiOptionItem *) items_.item(i);

        delete t;
    }
}

void TUiOption::add( const char * name, unsigned char value )
{
    items_.add( new TUiOptionItem(name,value) );
}

const char * TUiOption::item( int index ) const
{
    TUiOptionItem * t = (TUiOptionItem *) items_.item(index);

    return (t != 0) ? t->name_ : 0;
}

unsigned char TUiOption::itemValue( int index ) const
{
    TUiOptionItem * t = (TUiOptionItem *) items_.item(index);

    return (t != 0) ? t->value_ : 0;
}

void TUiOption::setItemName( int index, const char * name )
{
    TUiOptionItem * t = (TUiOptionItem *) items_.item(index);

    if( t != 0 ) {
        t->name_ = name;
    }
}

void TUiOption::select( int index )
{
    if( (index >= 0) && (index < count()) && (index != selected()) ) {
        selected_ = index;
    }
}

void TUiOption::notify( TMachine * machine )
{
    machine->handleInputEvent( id(), selected() );
}

TUiOption * TUiOptionGroup::add( unsigned id, const char * name, int def, unsigned char mask ) 
{
    TUiOption * result = new TUiOption( type_ | id, name, def, mask );
    items_.add( result );
    return result;
}

bool TUiOptionGroup::remove( unsigned id )
{
    bool result = false;

    id |= type_;

    for( int i=0; i<count(); i++ ) {
        if( item(i)->id() == id ) {
            delete item(i);
            items_.remove( i );
            result = true;
            break;
        }
    }

    return result;
}

TUiOptionGroup::~TUiOptionGroup()
{
    for( int i=0; i<count(); i++ ) {
        delete item(i);
    }
}

void TUiOptionGroup::notify( TMachine * machine )
{
    for( int i=0; i<count(); i++ ) {
        item(i)->notify( machine );
    }
}

TUserInterface::~TUserInterface() {
    clear();
}

TUiOptionGroup * TUserInterface::addGroup( const char * name, unsigned type ) 
{
    TUiOptionGroup * result = new TUiOptionGroup( name, type );
    items_.add( result );
    return result;
}

TUiOptionGroup * TUserInterface::group( const char * name )
{
    TUiOptionGroup * result = 0;

    for( int i=0; i<groupCount(); i++ ) {
        if( strcmp( group(i)->name(), name ) == 0 ) {
            result = group(i);
            break;
        }
    }

    return result;
}

void TUserInterface::notify( TMachine * machine )
{
    for( int i=0; i<groupCount(); i++ ) {
        group(i)->notify( machine );
    }
}

void TUserInterface::removeOption( unsigned id )
{
    for( int i=0; i<groupCount(); i++ ) {
        group(i)->remove( id );
    }
}

TUiOption * TUserInterface::findOption( unsigned id )
{
    TUiOption * result = 0;

    for( int i=0; i<groupCount() && result==0; i++ ) {
        TUiOptionGroup * g = group( i );

        for( int j=0; j<g->count(); j++ ) {
            TUiOption * option = g->item( j );

            if( id == TInput::getDeviceId( option->id() ) ) {
                result = option;
                break;
            }
        }
    }

    return result;
}

TUiOption * TUserInterface::replaceOption( unsigned id, const char * name, int def, unsigned char mask )
{
    TUiOption * result = 0;

    for( int i=0; i<groupCount(); i++ ) {
        TUiOptionGroup * g = group( i );

        if( g->remove( id ) ) {
            result = g->add( id, name, def, mask );
            break;
        }
    }

    return result;
}

void TUserInterface::renameOption( unsigned id, const char * name )
{
    TUiOption * option = findOption( id );

    if( option != 0 ) {
        option->setName( name );
    }
}

void TUserInterface::renameOptionItem( unsigned id, int index, const char * name )
{
    TUiOption * option = findOption( id );

    if( option != 0 ) {
        option->setItemName( index, name );
    }
}

void TUserInterface::setOptionDefault( unsigned id, int def, bool select )
{
    TUiOption * option = findOption( id );

    if( option != 0 ) {
        option->setDefault( def );
        if( select ) {
            option->restoreDefault();
        }
    }
}

void TUserInterface::clear()
{
    for( int i=0; i<groupCount(); i++ ) {
        delete group(i);
    }

    items_.clear();
}
