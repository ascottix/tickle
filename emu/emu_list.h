/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef EMU_LIST_H_
#define EMU_LIST_H_

class TList {
public:
    TList();

    /** Destructor. */
    virtual ~TList();

    int add( void * item );

    void clear();

    void remove( int index );

    void exchange( int index1, int index2 );

    int indexOf( void * item ) const;

    void insert( int index, void * item );

    void * item( int index ) const;

    void * operator [] (int index) const {
        return item( index );
    }

    int count() const {
        return count_;
    }

private:
    TList( const TList & list );
    TList & operator = ( const TList & list );

    void expand();

    void ** items_;
    int capacity_;
    int count_;
};

#endif // EMU_LIST_H_
