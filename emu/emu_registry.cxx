/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include <stdio.h>
#include <string.h>

#include "emu_registry.h"

TGameRegistry::~TGameRegistry()
{
    for( int i=0; i<count(); i++ ) {
        TGameRegistryItem * t = (TGameRegistryItem *) entries_[i];
        delete t;
    }
}

void TGameRegistry::add( TGameRegistryItem * item )
{
    entries_.add( item );
}

const TGameRegistryItem * TGameRegistry::item( int index ) const {
    TGameRegistryItem * result = 0;
    
    if( index >= 0 && index < count() ) {
        result = (TGameRegistryItem *) entries_[ index ];
    }
    
    return result;
}

int TGameRegistry::find( const char * name ) const
{
    int index;
    
    for( index=count()-1; index>=0; index-- ) {
        if( ! strcmp( item(index)->info()->driver, name ) || ! strcmp( item(index)->name(), name ) ) {
            break;
        }
    }

    return index;
}

void TGameRegistry::sort()
{
    int n = count();

    for( int i=0; i<n-1; i++ ) {
        int k = i;

        for( int j=i+1; j<n; j++ ) {
            if( strcmp( item(j)->name(), item(k)->name() ) < 0 ) {
                k = j;
            }
        }

        if( k != i ) {
            entries_.exchange( i, k );
        }
    }
}

TString TGameRegistry::getAsJson()
{
    TString s;
    
    s.append( "[\n" );
    
    for( int i=0; i<count(); i++ ) {
        const TGameRegistryItem * item = (TGameRegistryItem *) entries_[i];
        const TMachineInfo * info = item->info();
        TMachine * machine = TMachine::createInstance( item->factory() );
        
        TString resources;

        for( int j=0; j<machine->getResourceCount(); j++ ) {
            if( j > 0 ) resources.append( ", " );
            resources.append( "\"" );
            resources.append( machine->getResourceName( j ) );
            resources.append( "\"" );
        }
        
        char buffer[1024];
        
        sprintf( buffer, "\t{\n"
                "\t\t\"driver\": \"%s\",\n"
                "\t\t\"name\": \"%s\",\n"
                "\t\t\"manufacturer\": \"%s\",\n"
                "\t\t\"resources\": [%s],\n"
                "\t\t\"year\": %d,\n"
                "\t\t\"width\": %d,\n"
                "\t\t\"height\": %d\n"
                "\t}",
                info->driver, info->name, info->manufacturer, resources.cstr(), info->year, info->screenWidth, info->screenHeight );
        
        s.append( buffer );
        if( (i+1) < count() ) s.append( "," );
        s.append( "\n");
        
        delete machine;
    }
    
    s.append( "]\n" );
    
    return s;
}

TGameRegistry & TGameRegistry::instance()
{
    static TGameRegistry theGameRegistry;

    return theGameRegistry;
}

TGameRegistrationHandler::TGameRegistrationHandler( const TMachineInfo * info, TMachineFactoryFunc factory )
{
    TGameRegistry::instance().add( new TGameRegistryItem( info, factory ) );
}
