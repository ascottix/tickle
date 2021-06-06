/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#ifndef EMU_PALETTE_H_
#define EMU_PALETTE_H_

enum TPaletteByteEncodingSchema 
{
    esDefault,
    esAlternate,
};

class TPalette
{
public:
    TPalette( int colors ) {
        colors_ = colors;
        data_ = new unsigned [colors];
    }

    /** Destructor. */
    virtual ~TPalette() {
        delete data_;
    }

    unsigned color( int index ) const {
        return data_[index];
    }

    void setColor( int index, unsigned color ) {
        if( (index >= 0) && (index < colors_) ) {
            data_[index] = color;
        }
    }

    unsigned colors() const {
        return colors_;
    }

    unsigned * data() {
        return data_;
    }

    int getNearestColor( unsigned color );

    static unsigned encodeColor( unsigned r, unsigned g, unsigned b );

    static void decodeColor( unsigned color, unsigned char * r, unsigned char * g, unsigned char * b );

    static unsigned decodeByte( unsigned char value, int schema = esDefault );

private:
    TPalette( const TPalette & );
    const TPalette & operator = ( const TPalette & );

    int colors_;
    unsigned * data_;
};

#endif // EMU_PALETTE_H_
