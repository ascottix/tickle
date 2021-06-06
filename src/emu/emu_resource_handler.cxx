/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#include "emu_machine.h"
#include "emu_resource_handler.h"

struct TResourceHandlerItem : public TResourceFileInfo
{
    TResourceHandlerItem( int i, const char * n, unsigned c, unsigned s, TResourceFileType t, unsigned char * b );

    unsigned char * buf;
};

TResourceHandlerItem::TResourceHandlerItem( int i, const char * n, unsigned c, unsigned s, TResourceFileType t, unsigned char * b )
    : TResourceFileInfo( i, n, c, s, t ), buf( b )
{
}

TResourceHandler::~TResourceHandler()
{
    clear();
}

void TResourceHandler::clear()
{
    for( int i=0; i<items_.count(); i++ ) {
        TResourceHandlerItem * item = (TResourceHandlerItem *) items_.item(i);

        delete item;
    }

    items_.clear();
}

void TResourceHandler::add( int id, const char * name, unsigned len, TResourceFileType type, unsigned char * buf, unsigned crc )
{
    remove( id );

    TResourceHandlerItem * item = new TResourceHandlerItem( id, name, crc, len, type, buf );

    items_.add( item );
}

void TResourceHandler::replace( int id, const char * name, unsigned crc, unsigned char * buf )
{
    for( int i=0; i<items_.count(); i++ ) {
        TResourceHandlerItem * item = (TResourceHandlerItem *) items_.item(i);

        if( item->id == id ) {
            item->name = name;
            item->crc = crc;
            if( buf != 0 ) {
                item->buf = buf;
            }
            break;
        }
    }
}

void TResourceHandler::replace( TResourceHandlerReplaceInfo * info, int count )
{
    while( 1 ) {
        replace( info->id, info->name, info->crc );

        count--;

        if( count == 0 )
            break;

        info++;

        if( info->name == 0 )
            break;
    }
}

void TResourceHandler::remove( int id )
{
    for( int i=0; i<items_.count(); i++ ) {
        TResourceHandlerItem * item = (TResourceHandlerItem *) items_.item(i);

        if( item->id == id ) {
            items_.remove( i );
            delete item;
            break;
        }
    }
}

void TResourceHandler::addToMachineDriverInfo( TMachineDriverInfo * di )
{
    for( int i=0; i<items_.count(); i++ ) {
        TResourceHandlerItem * item = (TResourceHandlerItem *) items_.item(i);

        di->addResourceFile( item->id, item->name, item->crc, item->size, item->type );
    }
}

int TResourceHandler::handle( int id, const unsigned char * buf, unsigned len )
{
    int result = -1;

    for( int i=0; i<items_.count(); i++ ) {
        TResourceHandlerItem * item = (TResourceHandlerItem *) items_.item(i);

        if( item->id == id ) {
            if( TMachine::copyResourceFile( item->buf, item->size, buf, len ) ) {
                result = 0;
            }

            break;
        }
    }

    return result;
}
