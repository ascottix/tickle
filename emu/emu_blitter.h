/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef EMU_BLITTER_H_
#define EMU_BLITTER_H_

struct TBitBlitter
{
    virtual ~TBitBlitter () {
    }
    
    virtual void blit( unsigned char * dst, unsigned char * src, int len ) = 0;
};

struct TBltCopy : public TBitBlitter 
{
    virtual void blit( unsigned char * dst, unsigned char * src, int len ) {
        memcpy( dst, src, (size_t)len );
    }
};

struct TBltCopyReverse : public TBitBlitter
{
    virtual void blit( unsigned char * dst, unsigned char * src, int len ) {
        for( ; len > 0; len-- ) {
            *dst++ = *--src;
        }
    }
};

struct TBltAdd : public TBitBlitter 
{
    TBltAdd( unsigned char color ) : color_(color) {
    }

    virtual void blit( unsigned char * dst, unsigned char * src, int len ) {
        for( ; len > 0; len-- ) {
            *dst++ = color_ + *src++;
        }
    }

    TBltAdd * color( unsigned char c ) {
        color_ = c;
        return this;
    }

protected:
    unsigned char color_;
};

struct TBltAddReverse : public TBltAdd
{
    TBltAddReverse( unsigned char color ) : TBltAdd(color) {
    }

    virtual void blit( unsigned char * dst, unsigned char * src, int len ) {
        for( ; len > 0; len-- ) {
            *dst++ = color_ + *--src;
        }
    }
};

struct TBltAddSrcTrans : public TBltAdd
{
    TBltAddSrcTrans( unsigned char color, unsigned char trans ) : TBltAdd(color), trans_(trans) {
    }

    virtual void blit( unsigned char * dst, unsigned char * src, int len ) {
        for( ; len > 0; len-- ) {
            unsigned char c = *src++;
            if( c != trans_ ) *dst = color_ + c;
            dst++;
        }
    }

protected:
    unsigned char trans_;
};

struct TBltAddSrcTransReverse : public TBltAddSrcTrans
{
    TBltAddSrcTransReverse( unsigned char color, unsigned char trans ) : TBltAddSrcTrans(color,trans) {
    }

    virtual void blit( unsigned char * dst, unsigned char * src, int len ) {
        for( ; len > 0; len-- ) {
            unsigned char c = *--src;
            if( c != trans_ ) *dst = color_ + c;
            dst++;
        }
    }
};

struct TBltAddSrcZeroTransBlackAndWhite : public TBltAdd
{
    TBltAddSrcZeroTransBlackAndWhite( unsigned char color ) : TBltAdd(color) {
    }
    
    virtual void blit( unsigned char * dst, unsigned char * src, int len ) {
        for( ; len > 0; len-- ) {
            unsigned char c = *src++;
            if( c ) *dst = color_;
            dst++;
        }
    }
};


struct TBltAddSrcZeroTrans : public TBltAdd
{
    TBltAddSrcZeroTrans( unsigned char color ) : TBltAdd(color) {
    }

    virtual void blit( unsigned char * dst, unsigned char * src, int len ) {
        for( ; len > 0; len-- ) {
            unsigned char c = *src++;
            if( c ) *dst = color_ + c;
            dst++;
        }
    }
};

struct TBltAddSrcZeroTransReverse : public TBltAddSrcZeroTrans
{
    TBltAddSrcZeroTransReverse( unsigned char color ) : TBltAddSrcZeroTrans(color) {
    }

    virtual void blit( unsigned char * dst, unsigned char * src, int len ) {
        for( ; len > 0; len-- ) {
            unsigned char c = *--src;
            if( c ) *dst = color_ + c;
            dst++;
        }
    }
};

struct TBltAddXlat : public TBltAdd
{
    TBltAddXlat( unsigned char color, unsigned char * xlat ) : TBltAdd(color), xlat_(xlat) {
    }

    virtual void blit( unsigned char * dst, unsigned char * src, int len ) {
        for( ; len > 0; len-- ) {
            *dst++ = xlat_[ (unsigned char)(color_ + *src++) ];
        }
    }

protected:
    unsigned char * xlat_;
};

struct TBltAddXlatReverse : public TBltAddXlat
{
    TBltAddXlatReverse( unsigned char color, unsigned char * xlat ) : TBltAddXlat(color,xlat) {
    }

    virtual void blit( unsigned char * dst, unsigned char * src, int len ) {
        for( ; len > 0; len-- ) {
            *dst++ = xlat_[ (unsigned char)( color_ + *--src) ];
        }
    }
};

struct TBltAddXlatSrcTrans : public TBltAddSrcTrans
{
    TBltAddXlatSrcTrans( unsigned char color, unsigned char trans, unsigned char * xlat ) : TBltAddSrcTrans(color,trans), xlat_(xlat) {
    }

    virtual void blit( unsigned char * dst, unsigned char * src, int len ) {
        for( ; len > 0; len-- ) {
            unsigned char c = *src++;
            if( c != trans_ ) *dst = xlat_[ (unsigned char)( color_ + c) ];
            dst++;
        }
    }

protected:
    unsigned char * xlat_;
};

struct TBltAddXlatSrcTransReverse : public TBltAddSrcTrans
{
    TBltAddXlatSrcTransReverse( unsigned char color, unsigned char trans, unsigned char * xlat ) : TBltAddSrcTrans(color,trans), xlat_(xlat) {
    }

    virtual void blit( unsigned char * dst, unsigned char * src, int len ) {
        for( ; len > 0; len-- ) {
            unsigned char c = *--src;
            if( c != trans_ ) *dst = xlat_[ (unsigned char)( color_ + c) ];
            dst++;
        }
    }

protected:
    unsigned char * xlat_;
};

struct TBltAddXlatDstTrans : public TBltAddXlatSrcTrans
{
    TBltAddXlatDstTrans( unsigned char color, unsigned char trans, unsigned char * xlat ) : TBltAddXlatSrcTrans(color,trans,xlat) {
    }

    virtual void blit( unsigned char * dst, unsigned char * src, int len ) {
        for( ; len > 0; len-- ) {
            unsigned char c = xlat_[ (unsigned char)( color_ + *src++) ];
            if( c != trans_ ) *dst = c;
            dst++;
        }
    }
};

struct TBltAddXlatDstTransReverse : public TBltAddXlatDstTrans
{
    TBltAddXlatDstTransReverse( unsigned char color, unsigned char trans, unsigned char * xlat ) : TBltAddXlatDstTrans(color,trans,xlat) {
    }

    virtual void blit( unsigned char * dst, unsigned char * src, int len ) {
        for( ; len > 0; len-- ) {
            unsigned char c = xlat_[ (unsigned char)( color_ + *--src) ];
            if( c != trans_ ) *dst = c;
            dst++;
        }
    }
};

#endif // EMU_BLITTER_H_
