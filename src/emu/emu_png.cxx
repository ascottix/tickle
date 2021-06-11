/*
    Tickle class library
    PNG image decoder

    Copyright (c) 2004 Alessandro Scotti
*/
#include <assert.h>
#include <string.h>

#include <zlib.h>

#include "emu_crc32.h"
#include "emu_math.h"
#include "emu_png.h"

class TPngReader
{
public:
    TPngReader( TInputStream * );

    ~TPngReader();

    TBitmap * read();

private:
    void expandCurrentScanline( unsigned x, unsigned y, unsigned dx );
    void filterCurrentScanline();
    void flushCurrentScanline();

    bool handleImageData( unsigned char * data, unsigned size );
    bool handleImageHeader( unsigned char * data, unsigned size );
    bool handlePalette( unsigned char * data, unsigned size );

    TInputStream *  infile_;
    TBitmap *       bitmap_;
    TPalette *      palette_;
    unsigned        width_;
    unsigned        height_;
    unsigned        bits_channel_;
    unsigned        bpp_;
    unsigned        bytes_line_;
    unsigned        channels_;
    unsigned        color_type_;
    unsigned        compr_type_;
    unsigned        filter_type_;
    unsigned        lace_type_;
    unsigned char * old_scanline_;
    unsigned char * cur_scanline_;
    unsigned        cur_scanline_offset_;
    unsigned        cur_scanline_index_;
    unsigned        cur_lace_pass_;
    unsigned        cur_bytes_line_;
    z_stream        zs_;
};

const unsigned char PngSignature[] = { 137, 80, 78, 71, 13, 10, 26, 10 };

enum {
    CHUNK_IHDR = 0x49484452,       // Image header
    CHUNK_IDAT = 0x49444154,       // Image data
    CHUNK_IEND = 0x49454E44,       // Image end
    CHUNK_PLTE = 0x504C5445,       // Palette
    CHUNK_bKGD = 0x624B4744,       // Background
    CHUNK_cHRM = 0x6348524D,       // CIE color info of original display
    CHUNK_gAMA = 0x67414D41,       // Gamma
    CHUNK_hIST = 0x68495354,       // Color histogram
    CHUNK_oFFs = 0x6F464673,       // Device offset
    CHUNK_pCAL = 0x7043414C,       // Data transformation
    CHUNK_pHYs = 0x70485973,       // Physical device resolution
    CHUNK_sBIT = 0x73424954,       // Significant bits
    CHUNK_spAL = 0x7370414C,       // Suggested palette
    CHUNK_sRGB = 0x73524742,       //
    CHUNK_tEXt = 0x74455874,       // Text
    CHUNK_tIME = 0x74494D45,       // Time
    CHUNK_tRNS = 0x74524E53,       //
    CHUNK_zTXt = 0x7A545874,       // Compressed text

    LACE_NONE  = 0,               // Image not interlaced
    LACE_ADAM7 = 1,               // Image interlaced with the Adam7 method

    COLOR_MASK_PALETTE    = 1,    // Image has a palette
    COLOR_MASK_COLOR      = 2,    // Image has colors (as opposed to grayscale)
    COLOR_MASK_ALPHA      = 4,    // Image has alpha channel

    COLOR_TYPE_GRAY       = 0,
    COLOR_TYPE_PALETTE    = COLOR_MASK_COLOR | COLOR_MASK_PALETTE,
    COLOR_TYPE_RGB        = COLOR_MASK_COLOR,
    COLOR_TYPE_RGB_ALPHA  = COLOR_MASK_COLOR | COLOR_MASK_ALPHA,
    COLOR_TYPE_GRAY_ALPHA = COLOR_MASK_ALPHA,

    FILTER_NONE           = 0,
    FILTER_SUB            = 1,
    FILTER_UP             = 2,
    FILTER_AVERAGE        = 3,
    FILTER_PAETH          = 4,
};

const unsigned LaceXStart[] = { 0,4,0,2,0,1,0 };
const unsigned LaceXInc[]   = { 8,8,4,4,2,2,1 };
const unsigned LaceYStart[] = { 0,0,4,0,2,0,1 };
const unsigned LaceYInc[]   = { 8,8,8,4,4,2,2 };

TBitmap * createBitmapFromPNG( TInputStream * is )
{
    TPngReader reader( is );

    return reader.read();
}

TPngReader::TPngReader( TInputStream * is )
{
    // Initialize variables
    infile_ = is;
    bitmap_ = 0;
    palette_ = 0;
    old_scanline_ = 0;
    cur_scanline_ = 0;
    cur_scanline_offset_ = 0;
    cur_scanline_index_ = 0;

    // Initialize ZLIB decompressor
    memset( &zs_, 0, sizeof(zs_) );
    inflateInit( &zs_ );
}

TPngReader::~TPngReader()
{
    inflateEnd( &zs_ );

    delete [] old_scanline_;
    delete [] cur_scanline_;
}

static unsigned getUint32( const unsigned char * buf )
{
    return ((unsigned)buf[0] << 24) | ((unsigned)buf[1] << 16) | ((unsigned)buf[2] << 8) | (unsigned)buf[3];
}

void TPngReader::filterCurrentScanline()
{
    unsigned char filter = cur_scanline_[0];

    for( unsigned i=0; i<bytes_line_; i++ ) {
        unsigned char predicted = 0;
        unsigned char a = (i < bpp_) ? 0 : cur_scanline_[1+i-bpp_]; // Left
        unsigned char b = old_scanline_[i+bpp_]; // Upper
        unsigned char c = old_scanline_[i]; // Upper-left

        switch( filter ) {
        case FILTER_SUB:
            predicted = a;
            break;
        case FILTER_UP:
            predicted = b;
            break;
        case FILTER_AVERAGE:
            predicted = (a + b) / 2;
            break;
        case FILTER_PAETH:
            {
                int p = a + b - c;

                int da = TMath::abs( p - a );
                int db = TMath::abs( p - b );
                int dc = TMath::abs( p - c );

                if( (da <= db) && (da <= dc) )
                    predicted = a;
                else if( db <= dc )
                    predicted = b;
                else
                    predicted = c;

            }
            break;
        }

        cur_scanline_[1+i] = cur_scanline_[1+i] + predicted;
    }

    // Update last decoded scanline
    memcpy( old_scanline_+bpp_, cur_scanline_+1, bytes_line_ );
}

bool TPngReader::handleImageHeader( unsigned char * data, unsigned size )
{
    bool result = (size == 13);

    if( result ) {
        width_        = getUint32( data );
        height_       = getUint32( data+4 );
        bits_channel_ = data[8];
        color_type_   = data[9];
        compr_type_   = data[10];
        filter_type_  = data[11];
        lace_type_    = data[12];

        result = (width_ > 0) && (height_ > 0) && 
            (compr_type_ == 0) && 
            (filter_type_ == 0) && ((lace_type_ == LACE_NONE) || (lace_type_ == LACE_ADAM7));
    }

    if( result ) {
        switch( color_type_ ) {
        case COLOR_TYPE_GRAY:
            channels_ = 1;
            result = (bits_channel_ == 1) || (bits_channel_ == 2) || (bits_channel_ == 4) || (bits_channel_ == 8) || (bits_channel_ == 16);
            break;
        case COLOR_TYPE_PALETTE:
            channels_ = 1;
            result = (bits_channel_ == 1) || (bits_channel_ == 2) || (bits_channel_ == 4) || (bits_channel_ == 8);
            break;
        case COLOR_TYPE_RGB:
            channels_ = 3;
            result = (bits_channel_ == 8) || (bits_channel_ == 16);
            break;
        case COLOR_TYPE_GRAY_ALPHA:
            channels_ = 2;
            result = (bits_channel_ == 8) || (bits_channel_ == 16);
            break;
        case COLOR_TYPE_RGB_ALPHA:
            channels_ = 4;
            result = (bits_channel_ == 8) || (bits_channel_ == 16);
            break;
        default:
            result = false;
        }
    }

    if( result ) {
        bpp_ = ((channels_ * bits_channel_) + 7) / 8;
        bytes_line_ = ((width_ * channels_ * bits_channel_) + 7) / 8;

        old_scanline_ = new unsigned char [bytes_line_+bpp_];
        cur_scanline_ = new unsigned char [bytes_line_+1]; // Add one byte for filter type

        if( lace_type_ == LACE_ADAM7 ) {
            unsigned w = (width_ + LaceXInc[0] - 1 - LaceXStart[0]) / LaceXInc[0];

            cur_lace_pass_ = 0;
            cur_bytes_line_ = (w * channels_ * bits_channel_ + 7) / 8;
        }
        else {
            cur_bytes_line_ = bytes_line_;
        }

        memset( old_scanline_, 0, cur_bytes_line_ );

        if( ((color_type_ & (COLOR_MASK_PALETTE | COLOR_MASK_COLOR)) == 0) ) {
            // Allocate palette for grayscale image
            palette_ = new TPalette( (bits_channel_ >= 8) ? 256 : 1 << bits_channel_ );

            for( unsigned i=0; i<palette_->colors(); i++ ) {
                unsigned char c = (i*255) / (palette_->colors()-1);
                palette_->setColor( i, TPalette::encodeColor(c,c,c) );
            }
        }
    }

    return result;
}

bool TPngReader::handlePalette( unsigned char * data, unsigned size )
{
    bool result = false;

    if( (palette_ == 0) && ((size % 3) == 0) && (size > 0) && (size <= 3*256) ) {
        palette_ = new TPalette( size / 3 );

        for( unsigned i=0; i<palette_->colors(); i++ ) {
            palette_->setColor( i, TPalette::encodeColor(data[0],data[1],data[2]) );
            data += 3;
        }

        result = true;
    }
    // ...else extra palette chunk or invalid size

    return result;
}

TBitmap * TPngReader::read()
{
    bool success = false;

    unsigned char signature[ sizeof(PngSignature) ];

    int n = infile_->read( signature, sizeof(signature) );

    if( (n == sizeof(signature)) && (memcmp(signature,PngSignature,sizeof(signature)) == 0) ) {
        bool ok = true;

        while( ok ) {
            // Read and process chunk
            unsigned char header[8];

            if( infile_->read( header, sizeof(header) ) != sizeof(header) )
                break;

            unsigned size = getUint32( header+0 );
            unsigned type = getUint32( header+4 );
            unsigned char * data = 0;
            TCRC32 crc32;

            crc32.update( header+4, 4 );
            
            if( size > 0 ) {
                data = new unsigned char [ size ];            
                if( infile_->read( data, size ) != size ) {
                    ok = false;
                }
                else {
                    // Compute CRC
                    crc32.update( data, size );
                }
            }

            // Verify CRC
            if( infile_->read( header, 4 ) != 4 ) {
                ok = false;
            }
            else {
                ok = crc32.value() == getUint32( header );
            }

            // Process chunk
            if( ok ) {
                switch( type ) {
                case CHUNK_IHDR:
                    ok = handleImageHeader( data, size );
                    break;
                case CHUNK_IDAT:
                    ok = handleImageData( data, size );
                    break;
                case CHUNK_PLTE:
                    ok = handlePalette( data, size );
                    break;
                case CHUNK_IEND:
                    success = true;
                    ok = false; // Exit now, since this is the last chunk
                    break;
                default:
                    // Unknown chunk, make sure it's not critical
                    ok = (type & 0x20000000) != 0;
                    break;
                }
            }

            // Delete chunk data            
            delete [] data;
        }
    }
    // ...else not a PNG file

    // Delete resources if not successful
    if( ! success ) {
        delete palette_;
        delete bitmap_;
        bitmap_ = 0;
    }

    // Return bitmap (may be null)
    return bitmap_;
}

void TPngReader::flushCurrentScanline()
{
    // Done with this line, filter and output it
    filterCurrentScanline();

    if( lace_type_ == LACE_NONE ) {
        // No interlace
        expandCurrentScanline( 0, cur_scanline_index_, 1 );

        cur_scanline_index_++;
    }
    else {
        // Adam7 interlace
        expandCurrentScanline( LaceXStart[cur_lace_pass_], cur_scanline_index_, LaceXInc[cur_lace_pass_] );

        cur_scanline_index_ += LaceYInc[ cur_lace_pass_ ];

        if( cur_scanline_index_ >= height_ ) {
            cur_lace_pass_++;

            if( cur_lace_pass_ < 7 ) {
                unsigned w = (width_ + LaceXInc[cur_lace_pass_] - 1 - LaceXStart[cur_lace_pass_]) / LaceXInc[cur_lace_pass_];

                cur_scanline_index_ = LaceYStart[ cur_lace_pass_ ];
                cur_bytes_line_ = (w * channels_ * bits_channel_ + 7) / 8;
                memset( old_scanline_, 0, cur_bytes_line_ );
            }
        }
    }

    cur_scanline_offset_ = 0;
}

bool TPngReader::handleImageData( unsigned char * data, unsigned size )
{
    bool result = true;

    // Allocate bitmap if needed
    if( bitmap_ == 0 ) {
        if( color_type_ == COLOR_TYPE_GRAY  || color_type_ == COLOR_TYPE_GRAY_ALPHA || color_type_ == COLOR_TYPE_PALETTE ) {
            if( palette_ == 0 ) {
                result = false;
            }
            else {
                bitmap_ = new TBitmapIndexed( width_, height_, palette_ );
            }
        }
        else {
            bitmap_ = new TBitmapRGB( width_, height_ );
        }
    }

    if( result && (size > 0) ) {
        // Fill the input buffer
        zs_.avail_in = size;
        zs_.next_in = data;

        // Consume the decompressed output
        while( zs_.avail_in > 0 ) {
            // Resume from current position
            zs_.next_out = cur_scanline_ + cur_scanline_offset_;
            zs_.avail_out = cur_bytes_line_ + 1 - cur_scanline_offset_;

            // Expand
            int z = inflate( &zs_, Z_PARTIAL_FLUSH );

            if( (z != Z_OK) && (z != Z_STREAM_END) ) {
                result = false;
                break;
            }

            if( zs_.avail_out == 0 ) {
                flushCurrentScanline();
            }
            else {
                assert( zs_.avail_in == 0 );

                // Break at this point, we'll resume at the next chunk
                cur_scanline_offset_ = cur_bytes_line_ + 1 - zs_.avail_out;
            }
        }
    }   

    return result;
}

void TPngReader::expandCurrentScanline( unsigned x, unsigned y, unsigned dx )
{
    static int bpp_mask[4] = { 0x80, 0xC0, 0x00, 0xF0 };
    static int bpp_shift[4] = {   7,    6,    0,    4 };

    unsigned char * src = 1 + cur_scanline_;

    if( color_type_ == COLOR_TYPE_GRAY  || color_type_ == COLOR_TYPE_GRAY_ALPHA || color_type_ == COLOR_TYPE_PALETTE ) {
        // These formats map to an indexed bitmap (note that we can handle
        // grayscale here because we have properly initialized the palette)
        unsigned char * dst = ((TBitmapIndexed *)bitmap_)->bits()->data() + width_*y + x;

        if( bits_channel_ < 8 ) {
            unsigned char mask = 0;
            int shift;

            src--;

            while( x < width_ ) {
                // Reset mask and shift if needed
                if( mask == 0 ) {
                  mask = bpp_mask[ bits_channel_-1 ];
                  shift = bpp_shift[ bits_channel_-1 ];
                  src++;
                }

                // Get bit-packed index
                unsigned char d = (*src & mask) >> shift;

                // Write it and advance to next pixel
                *dst = d;

                x += dx;
                dst += dx;
                mask >>= bits_channel_;
                shift -= bits_channel_;
            }
        }
        else {
            // 8 or 16 bits per pixel (16 bit is only possible for gray)
            int s = (color_type_ & COLOR_MASK_ALPHA) ? bits_channel_ / 4 : bits_channel_ / 8;

            while( x < width_ ) {
                *dst = *src;

                src += s;
                x += dx;
                dst += dx;
            }
        }
    }
    else {
        // These formats map to a RGB bitmap
        unsigned * dst = ((TBitmapRGB *) bitmap_)->data() + width_*y + x;

        int b = bits_channel_ / 8;
        int s = b * ((color_type_ & COLOR_MASK_ALPHA) ? 4 : 3);

        while( x < width_ ) {
            *dst = TPalette::encodeColor( src[0], src[b], src[b*2] );

            src += s;
            x += dx;
            dst += dx;
        }
    }
}
