/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef EMU_CRC32_H_
#define EMU_CRC32_H_

class TCRC32
{
public:
    TCRC32( unsigned crc = 0 ) : crc_(crc) {
    }

    unsigned value() const {
        return crc_;
    }

    void reset() {
        crc_ = 0;
    }

    void update( const unsigned char * buf, int len );

private:
    unsigned crc_;
};

#endif // EMU_CRC32_H_
