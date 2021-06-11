/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#include <stdio.h>
#include <string.h>

#include <zlib.h>

#include "emu_string.h"
#include "emu_zipfile.h"

enum TZipMethod {
    zmStored = 0,
    zmDeflated = 8
};

TZipEntry::TZipEntry( char * name, unsigned size, unsigned csize, unsigned offset, unsigned crc, int method, TZipFile * owner ) 
{
    name_ = name;
    size_ = size;
    csize_ = csize;
    offset_ = offset;
    crc_ = crc;
    method_ = method;
    owner_ = owner;
}

TInputStream * TZipEntry::open() const
{
    return owner_->openEntry( this );
}

class TZipFileCopierStream : public TInputStream
{
public:
    unsigned read( void * buf, unsigned len );

private:
    friend class TZipFile;

    TZipFileCopierStream( FILE * f, unsigned csize ) : f_(f), csize_(csize) {
    }

    FILE * f_;
    unsigned csize_;
};

unsigned TZipFileCopierStream::read( void * buf, unsigned len )
{
    unsigned n = csize_ <= len ? csize_ : len;

    if( n > 0 ) {
        n = (unsigned) fread( buf, 1, n, f_ );

        if( n > 0 ) {
            csize_ -= n;
        }
    }

    return n;
}

class TZipFileInflaterStream : public TInputStream
{
public:
    virtual ~TZipFileInflaterStream();

    unsigned read( void * buf, unsigned len );

private:
    friend class TZipFile;

    TZipFileInflaterStream( FILE * f, unsigned csize );

    FILE * f_;
    unsigned char inbuf_[4*1024];
    unsigned csize_;
    z_stream zs_;
    int zerr_;
};

TZipFileInflaterStream::TZipFileInflaterStream( FILE * f, unsigned csize ) : f_(f), csize_(csize) 
{
    memset( &zs_, 0, sizeof(zs_) );

    zs_.next_in = inbuf_;
    zs_.avail_in = 0;

    inflateInit2( &zs_, -MAX_WBITS );

    zerr_ = Z_OK;
}

TZipFileInflaterStream::~TZipFileInflaterStream() 
{
    inflateEnd( &zs_ );
}

unsigned TZipFileInflaterStream::read( void * buf, unsigned len )
{
    if( zerr_ != Z_OK )
        return 0;

    if( len > 0 ) {
        zs_.next_out = (unsigned char *)buf;
        zs_.avail_out = len;

        while( zs_.avail_out > 0 ) {
            if( zs_.avail_in <= 0 ) {
                // Needs to fill the input buffer
                if( csize_ > 0 ) {
                    unsigned n = csize_ <= sizeof(inbuf_) ? csize_ : sizeof(inbuf_);
                    zs_.avail_in = (unsigned) fread( inbuf_, 1, n, f_ );
                }
               
                if( zs_.avail_in > 0 ) {
                    // Data read successfully, update counter
                    csize_ -= zs_.avail_in;
                    zs_.next_in = inbuf_;
                }
                else {
                    // Cannot read data, 
                    zerr_ = Z_STREAM_ERROR;
                }
            }

            // Break on error
            if( zerr_ != Z_OK )
                break;

            // Expand data
            zerr_ = ::inflate( &zs_, Z_NO_FLUSH );
        }
    }

    return len - zs_.avail_out;
}

TInputStream * TZipFile::openEntry( const TZipEntry * entry ) const
{
    TInputStream * result = 0;

    if( fseek( f_, entry->offset(), SEEK_SET ) == 0 ) {
        switch( entry->method() ) {
        case zmStored:
            result = new TZipFileCopierStream( f_, entry->csize() );
            break;
        case zmDeflated:
            result = new TZipFileInflaterStream( f_, entry->csize() );
            break;
        }
    }

    return result;
}

TZipFile * TZipFile::open( const char * name )
{
    TZipFile * result = 0;

    FILE * f = fopen( name, "rb" );

    if( f != 0 ) {
        result = new TZipFile( f );
    }

    return result;
}

TZipFile::TZipFile( FILE * f )
{
    f_ = f;

    while( 1 ) {
        // Read local header
        unsigned char header[30];

        if( fread( header, 4, 1, f_ ) != 1 ) {
            break;
        }

        // Skip over old signature if found
        if( header[0] == 'P' && header[1] == 'K' && header[2] == '0' && header[3] == '0' ) {
            fread( header, 4, 1, f_ );
        }

        // Read rest of header
        if( fread( header+4, sizeof(header)-4, 1, f_ ) != 1 )
            break;

        // Verify signature
        if( header[0] != 0x50 || header[1] != 0x4B || header[2] != 0x03 || header[3] != 0x04 )
            break;

        // Local header seems ok, get fields
        int method = (int)( (unsigned)header[8] + 256*(unsigned)header[9] );
        unsigned crc = (unsigned)header[14] + ((unsigned)header[15] << 8) + ((unsigned)header[16] << 16) + ((unsigned)header[17] << 24);
        unsigned csize = (unsigned)header[18] + ((unsigned)header[19] << 8) + ((unsigned)header[20] << 16) + ((unsigned)header[21] << 24);
        unsigned size = (unsigned)header[22] + ((unsigned)header[23] << 8) + ((unsigned)header[24] << 16) + ((unsigned)header[25] << 24);
        unsigned name_len = (unsigned)header[26] + 256*(unsigned)header[27];
        unsigned extra_len = (unsigned)header[28] + 256*(unsigned)header[29];

        // Read variable-length name
        char * name = new char [ 1+name_len ];
        if( fread( name, 1, name_len, f_ ) != name_len ) {
            delete [] name;
            break;
        }
        name[name_len] = '\0';

        // Skip extra field
        if( extra_len && fseek( f_, extra_len, SEEK_CUR ) != 0 ) {
            delete [] name;
            break;
        }

        // Add entry to list
        TZipEntry * e = new TZipEntry( name, size, csize, (unsigned) ftell(f_), crc, method, this );

        entries_.add( e );

        // Skip compressed data
        if( fseek( f_, csize, SEEK_CUR ) != 0 )
            break;            
    }
}

TZipFile::~TZipFile()
{
    fclose( f_ );

    for( int i=0; i<entries_.count(); i++ ) {
        TZipEntry * e = (TZipEntry *) entries_[i];
        delete e;
    }
}

const TZipEntry * TZipFile::entry( const char * name, bool nameonly )
{
    const TZipEntry * result = 0;

    TString zname( name );

    for( int i=0; i<count(); i++ ) {
        const TZipEntry * ze = entry(i);

        TString ename( ze->name() );

        if( nameonly ) {
            // Remove path information
            int slash = ename.lastpos( L'/' );

            if( slash > 0 ) {
                ename.remove( 0, slash+1 );
            }
        }
        
        if( zname.icmp( ename ) == 0 ) {
            result = ze;
            break;
        }
    }

    return result;
}
