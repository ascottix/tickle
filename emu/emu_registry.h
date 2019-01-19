/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef EMU_REGISTRY_H_
#define EMU_REGISTRY_H_

#include "emu_info.h"
#include "emu_list.h"
#include "emu_machine.h"

class TGameRegistryItem
{
public:
    TGameRegistryItem( const TMachineInfo * info, TMachineFactoryFunc factory ) : info_(info), factory_(factory) {
    }

    const char * name() const {
        return info_->name;
    }

    const TMachineInfo * info() const {
        return info_;
    }

    TMachineFactoryFunc factory() const {
        return factory_;
    }

private:
    const TMachineInfo * info_;
    TMachineFactoryFunc factory_;
};

class TGameRegistry
{
public:
    /** Destructor. */
    ~TGameRegistry();

    void add( TGameRegistryItem * item );

    int count() const {
        return entries_.count();
    }

    const TGameRegistryItem * item( int index ) const;

    int find( const char * name ) const;

    void sort();
    
    TString getAsJson();
    
    static TGameRegistry & instance();

private:
    TGameRegistry() {
    }

    TList entries_;
};

class TGameRegistrationHandler
{
public:
    TGameRegistrationHandler( const TMachineInfo * info, TMachineFactoryFunc factory );
};

#endif // EMU_REGISTRY_H_
