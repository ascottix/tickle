/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#include <stdlib.h> // Wide-character functions
#include <string.h>

#include <wctype.h>

#include "emu_string.h"

const wchar_t * EmptyString = L"";

TString::~TString()
{
    delete buf_;
    delete cstr_buf_;
}

TString & TString::assign( const char * s )
{
    if( s != 0 ) {
        size_t count = strlen( s );
        
        len_ = mbstowcs( 0, s, count );

        if( len_ == (size_t)-1 ) {
            assign( EmptyString );   
        }
        else {
            delete buf_;
            buf_ = new wchar_t [ 1 + len_ ];
            mbstowcs( buf_, s, count+1 );
        }
    }
    else {
        assign( EmptyString );
    }

    return *this;
}

TString & TString::assign( const wchar_t * s )
{
    delete buf_;
    buf_ = strdup( s != 0 ? s : EmptyString );
    len_ = wcslen( buf_ );

    return *this;
}

TString & TString::append( wchar_t c )
{
    wchar_t s[2];

    s[0] = c;
    s[1] = L'\0';

    return append( s );
}

TString & TString::append( const TString & s )
{
    wchar_t * b = new wchar_t [ len_ + s.len_ + 1 ];
    wcscpy( b, buf_ );
    wcscpy( b + len_, s.buf_ );

    delete buf_;
    buf_ = b;
    len_ += s.len_;

    return *this;
}

void TString::insert( const TString & s, int index )
{
    if( (index >= 0) && (index < (int)len_) ) {
        wchar_t * b = new wchar_t [ len_ + s.len_ + 1 ];

        wcsncpy( b, buf_, index );
        wcscpy( b+index, s.buf_ );
        wcscpy( b+index+s.len_, buf_+index );

        delete buf_;
        buf_ = b;
        len_ += s.len_;
    }
    else if( index == (int)len_ ) {
        append( s );
    }
}

void TString::remove( int index, int count )
{
    if( (index >= 0) && (index < (int)len_) ) {
        int idx2 = index + (count < 0 ? (int) len_ : count);

        if( idx2 > (int) len_ ) {
            idx2 = (int) len_;
        }

        int n = idx2 - index;

        if( n > 0 ) {
            len_ = (size_t) n;
            while( buf_[idx2] ) {
                buf_[index++] = buf_[idx2++];
            }
            buf_[index] = '\0';
        }
    }    
}

TString TString::copy( int index, int count ) const
{
    TString result;

    if( (index >= 0) && (index < (int)len_) ) {
        if( count < 0 ) {
            count = (int) len_;
        }

        while( (buf_[index] != L'\0') && (count > 0) ) {
            result.append( buf_[index++] );
            count--;
        }
    }

    return result;
}

int TString::pos( wchar_t c, int index ) const
{
    wchar_t * result = 0;
    
    if( (index >= 0) && (index < (int)len_) ) {
        result = wcschr( buf_+index, c );
    }

    return result == 0 ? -1 : (int) (result - buf_);
}

int TString::lastpos( wchar_t c ) const
{
    wchar_t * result = wcsrchr( buf_, c );

    return result == 0 ? -1 : (int) (result - buf_);
}

int TString::pos( const TString & s, int index ) const
{
    wchar_t * result = 0;
    
    if( (index >= 0) && (index < (int)len_) ) {
        result = wcsstr( buf_+index, s.buf_ );
    }

    return result == 0 ? -1 : (int) (result - buf_);
}

const char * TString::cstr() const
{
    delete cstr_buf_;

    size_t count = wcstombs( 0, buf_, len_*2+1 );

    if( count == (size_t)-1 ) {
        cstr_buf_ = strdup( "" );
    }
    else {
        count++;
        cstr_buf_ = new char [ count ];
        wcstombs( cstr_buf_, buf_, count );
    }

    return cstr_buf_;
}

int TString::cmp( const TString & s ) const
{
    return wcscmp( buf_, s.buf_ );
}

int TString::icmp( const TString & s ) const
{
    wchar_t * b1 = buf_;
    wchar_t * b2 = s.buf_;

    while( 1 ) {
        wint_t c1 = towlower( *b1++ );
        wint_t c2 = towlower( *b2++ );

        if( c1 < c2 )
            return -1;
        else if( c1 > c2 )
            return +1;
        else if( c1 == 0 )
            break;
    }

    return 0;
}

long TString::tol( int base ) const
{
    return wcstol( buf_, 0, base );
}

unsigned long TString::toul( int base ) const
{
    return wcstoul( buf_, 0, base );
}

char * TString::strdup( const char * s )
{
    char * result = 0;

    if( s != 0 ) {
        result = new char [ 1 + strlen(s) ];
        strcpy( result, s );
    }

    return result;
}

wchar_t * TString::strdup( const wchar_t * s )
{
    wchar_t * result = 0;

    if( s != 0 ) {
        result = new wchar_t [ 1 + wcslen(s) ];
        wcscpy( result, s );
    }

    return result;
}
