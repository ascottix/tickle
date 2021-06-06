/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#ifndef EMU_EVENT2PORT_H_
#define EMU_EVENT2PORT_H_

enum TEventToPortItemType 
{
    ptNormal,
    ptInverted
};

class TEventToPortItem;

class TEventToPortHandler
{
public:
    TEventToPortHandler();

    /** Destructor. */
    ~TEventToPortHandler();

    void add( unsigned device, int type, unsigned char * port, unsigned char and_mask, unsigned char or_mask );

    void add( unsigned device, int type, unsigned char * port, unsigned char and_mask ) {
        add( device, type, port, and_mask, and_mask );
    }

    bool remove( unsigned device );

    bool handleInputEvent( unsigned device, unsigned param );

private:
    void expand();

    int count_;
    int capacity_;
    TEventToPortItem * items_;
};

#endif // EMU_EVENT2PORT_H_
