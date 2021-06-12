/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#include <assert.h>
#include <stdio.h>

#include "emu_memory_iostream.h"
#include "emu_png.h"

#include "emu_tickle_image.h"
#include "emu_tickle_machine.h"

unsigned char fontdata[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // #0
    0x7E, 0x81, 0xA5, 0x81, 0xA5, 0x99, 0x81, 0x7E, // #1
    0x7E, 0xFF, 0xDB, 0xFF, 0xDB, 0xE7, 0xFF, 0x7E, // #2
    0x6C, 0xFE, 0xFE, 0xFE, 0x7C, 0x38, 0x10, 0x00, // #3
    0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x10, 0x00, // #4
    0x10, 0x38, 0x10, 0x54, 0xFE, 0x54, 0x10, 0xFE, // #5
    0x10, 0x38, 0x7C, 0xFE, 0xFE, 0x7C, 0x10, 0xFE, // #6
    0x00, 0x18, 0x3C, 0x7E, 0x7E, 0x3C, 0x18, 0x00, // #7
    0xFF, 0xE7, 0xC3, 0x81, 0x81, 0xC3, 0xE7, 0xFF, // #8
    0x00, 0x3C, 0x66, 0x42, 0x42, 0x66, 0x3C, 0x00, // #9
    0xFF, 0xC3, 0x99, 0xBD, 0xBD, 0x99, 0xC3, 0xFF, // #10
    0x07, 0x03, 0x05, 0x78, 0x84, 0x84, 0x84, 0x78, // #11
    0x7C, 0x82, 0x82, 0x82, 0x7C, 0x10, 0x38, 0x10, // #12
    0x1C, 0x10, 0x1C, 0x10, 0x10, 0x10, 0x30, 0x30, // #13
    0x3E, 0x22, 0x3E, 0x22, 0x22, 0x26, 0x66, 0x60, // #14
    0x99, 0x5A, 0x3C, 0xE7, 0xE7, 0x3C, 0x5A, 0x99, // #15
    0x00, 0x10, 0x30, 0x70, 0xF0, 0x70, 0x30, 0x10, // #16
    0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xE0, 0xC0, 0x80, // #17
    0x10, 0x38, 0x54, 0x10, 0x10, 0x54, 0x38, 0x10, // #18
    0x48, 0x48, 0x48, 0x48, 0x48, 0x00, 0x48, 0x00, // #19
    0x7E, 0x92, 0x92, 0x72, 0x12, 0x12, 0x12, 0x00, // #20
    0x3C, 0x22, 0x18, 0x24, 0x24, 0x18, 0x44, 0x3C, // #21
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x3E, 0x00, // #22
    0x38, 0x54, 0x10, 0x10, 0x10, 0x54, 0x38, 0xFE, // #23
    0x00, 0x10, 0x38, 0x54, 0x10, 0x10, 0x10, 0x00, // #24
    0x00, 0x10, 0x10, 0x10, 0x54, 0x38, 0x10, 0x00, // #25
    0x00, 0x08, 0x04, 0xFE, 0x04, 0x08, 0x00, 0x00, // #26
    0x00, 0x20, 0x40, 0xFE, 0x40, 0x20, 0x00, 0x00, // #27
    0x00, 0x00, 0x80, 0x80, 0x80, 0xFC, 0x00, 0x00, // #28
    0x00, 0x24, 0x42, 0xFF, 0x42, 0x24, 0x00, 0x00, // #29
    0x00, 0x00, 0x10, 0x38, 0x7C, 0xFE, 0x00, 0x00, // #30
    0x00, 0x00, 0xFE, 0x7C, 0x38, 0x10, 0x00, 0x00, // #31
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  
    0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x10, 0x00, // !
    0x00, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, // "
    0x24, 0x24, 0x7E, 0x24, 0x7E, 0x24, 0x24, 0x00, // #
    0x38, 0x54, 0x50, 0x38, 0x14, 0x54, 0x38, 0x10, // $
    0x00, 0x02, 0x44, 0x08, 0x10, 0x20, 0x42, 0x00, // %
    0x38, 0x44, 0x38, 0x60, 0x94, 0x88, 0x74, 0x00, // &
    0x20, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, // '
    0x10, 0x20, 0x40, 0x40, 0x40, 0x20, 0x10, 0x00, // (
    0x40, 0x20, 0x10, 0x10, 0x10, 0x20, 0x40, 0x00, // )
    0x00, 0x24, 0x18, 0x7E, 0x18, 0x24, 0x00, 0x00, // *
    0x00, 0x10, 0x10, 0x7C, 0x10, 0x10, 0x00, 0x00, // +
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x20, // ,
    0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, // -
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, // .
    0x00, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, // /
    0x7C, 0xC6, 0x8A, 0x92, 0xA2, 0xC6, 0x7C, 0x00, // 0
    0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, // 1
    0x78, 0x84, 0x04, 0x18, 0x60, 0x80, 0xFC, 0x00, // 2
    0x78, 0x84, 0x04, 0x38, 0x04, 0x84, 0x78, 0x00, // 3
    0x1C, 0x24, 0x44, 0x84, 0xFE, 0x04, 0x0E, 0x00, // 4
    0xFC, 0x80, 0xF8, 0x04, 0x04, 0x84, 0x78, 0x00, // 5
    0x78, 0x84, 0x80, 0xF8, 0x84, 0x84, 0x78, 0x00, // 6
    0xFC, 0x04, 0x04, 0x08, 0x10, 0x20, 0x20, 0x00, // 7
    0x78, 0x84, 0x84, 0x78, 0x84, 0x84, 0x78, 0x00, // 8
    0x78, 0x84, 0x84, 0x7C, 0x04, 0x84, 0x78, 0x00, // 9
    0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, // :
    0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x10, 0x20, // ;
    0x08, 0x10, 0x20, 0x40, 0x20, 0x10, 0x08, 0x00, // <
    0x00, 0x00, 0xFC, 0x00, 0x00, 0xFC, 0x00, 0x00, // =
    0x40, 0x20, 0x10, 0x08, 0x10, 0x20, 0x40, 0x00, // >
    0x78, 0x84, 0x04, 0x08, 0x10, 0x00, 0x10, 0x00, // ?
    0x7C, 0x82, 0xBA, 0xA6, 0xBE, 0x80, 0x7C, 0x00, // @
    0x78, 0x84, 0x84, 0xFC, 0x84, 0x84, 0x84, 0x00, // A
    0xF8, 0x84, 0x84, 0xF8, 0x84, 0x84, 0xF8, 0x00, // B
    0x78, 0x84, 0x80, 0x80, 0x80, 0x84, 0x78, 0x00, // C
    0xF0, 0x88, 0x84, 0x84, 0x84, 0x88, 0xF0, 0x00, // D
    0xFC, 0x80, 0x80, 0xF0, 0x80, 0x80, 0xFC, 0x00, // E
    0xFC, 0x80, 0x80, 0xF0, 0x80, 0x80, 0x80, 0x00, // F
    0x78, 0x84, 0x80, 0x9C, 0x84, 0x84, 0x78, 0x00, // G
    0x84, 0x84, 0x84, 0xFC, 0x84, 0x84, 0x84, 0x00, // H
    0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, // I
    0x1C, 0x08, 0x08, 0x08, 0x88, 0x88, 0x70, 0x00, // J
    0x84, 0x88, 0x90, 0xE0, 0x90, 0x88, 0x84, 0x00, // K
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFC, 0x00, // L
    0xC6, 0xAA, 0x92, 0x82, 0x82, 0x82, 0x82, 0x00, // M
    0x82, 0xC2, 0xA2, 0x92, 0x8A, 0x86, 0x82, 0x00, // N
    0x78, 0x84, 0x84, 0x84, 0x84, 0x84, 0x78, 0x00, // O
    0xF8, 0x84, 0x84, 0xF8, 0x80, 0x80, 0x80, 0x00, // P
    0x78, 0x84, 0x84, 0x84, 0x94, 0x88, 0x76, 0x00, // Q
    0xF8, 0x84, 0x84, 0xF8, 0x90, 0x88, 0x84, 0x00, // R
    0x78, 0x84, 0x80, 0x78, 0x04, 0x84, 0x78, 0x00, // S
    0xFE, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, // T
    0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x78, 0x00, // U
    0x84, 0x84, 0x84, 0x84, 0x84, 0x48, 0x30, 0x00, // V
    0x82, 0x82, 0x82, 0x82, 0x92, 0xAA, 0xC6, 0x00, // W
    0x82, 0x44, 0x28, 0x10, 0x28, 0x44, 0x82, 0x00, // X
    0x44, 0x44, 0x44, 0x38, 0x10, 0x10, 0x10, 0x00, // Y
    0xFE, 0x04, 0x08, 0x10, 0x20, 0x40, 0xFE, 0x00, // Z
    0x78, 0x40, 0x40, 0x40, 0x40, 0x40, 0x78, 0x00, // [
    0x00, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x00, // \ (backslash)
    0x78, 0x08, 0x08, 0x08, 0x08, 0x08, 0x78, 0x00, // ]
    0x10, 0x28, 0x44, 0x82, 0x00, 0x00, 0x00, 0x00, // ^
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, // _
    0x20, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, // `
    0x00, 0x00, 0x38, 0x04, 0x3C, 0x44, 0x7C, 0x00, // a
    0x00, 0x40, 0x40, 0x78, 0x44, 0x44, 0x78, 0x00, // b
    0x00, 0x00, 0x3C, 0x40, 0x40, 0x40, 0x3C, 0x00, // c
    0x00, 0x04, 0x04, 0x3C, 0x44, 0x44, 0x3C, 0x00, // d
    0x00, 0x00, 0x38, 0x44, 0x7C, 0x40, 0x3C, 0x00, // e
    0x00, 0x0C, 0x10, 0x3C, 0x10, 0x10, 0x10, 0x00, // f
    0x00, 0x00, 0x3C, 0x44, 0x44, 0x3C, 0x04, 0x38, // g
    0x00, 0x40, 0x40, 0x78, 0x44, 0x44, 0x44, 0x00, // h
    0x00, 0x10, 0x00, 0x10, 0x10, 0x10, 0x10, 0x00, // i
    0x00, 0x04, 0x00, 0x04, 0x04, 0x04, 0x44, 0x38, // j
    0x00, 0x40, 0x40, 0x50, 0x60, 0x50, 0x48, 0x00, // k
    0x00, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, // l
    0x00, 0x00, 0x68, 0x54, 0x54, 0x44, 0x44, 0x00, // m
    0x00, 0x00, 0x78, 0x44, 0x44, 0x44, 0x44, 0x00, // n
    0x00, 0x00, 0x38, 0x44, 0x44, 0x44, 0x38, 0x00, // o
    0x00, 0x00, 0x78, 0x44, 0x44, 0x78, 0x40, 0x40, // p
    0x00, 0x00, 0x3C, 0x44, 0x44, 0x3C, 0x04, 0x04, // q
    0x00, 0x00, 0x5C, 0x60, 0x40, 0x40, 0x40, 0x00, // r
    0x00, 0x00, 0x38, 0x40, 0x7C, 0x04, 0x78, 0x00, // s
    0x00, 0x10, 0x38, 0x10, 0x10, 0x10, 0x18, 0x00, // t
    0x00, 0x00, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00, // u
    0x00, 0x00, 0x44, 0x44, 0x44, 0x28, 0x10, 0x00, // v
    0x00, 0x00, 0x44, 0x44, 0x54, 0x54, 0x6C, 0x00, // w
    0x00, 0x00, 0x44, 0x28, 0x10, 0x28, 0x44, 0x00, // x
    0x00, 0x00, 0x44, 0x44, 0x44, 0x3C, 0x04, 0x78, // y
    0x00, 0x00, 0x7C, 0x04, 0x38, 0x40, 0x7C, 0x00, // z
    0x00, 0x08, 0x10, 0x10, 0x30, 0x10, 0x10, 0x08, // {
    0x00, 0x10, 0x10, 0x10, 0x00, 0x10, 0x10, 0x10, // |
    0x00, 0x20, 0x10, 0x10, 0x18, 0x10, 0x10, 0x20, // }
    0x64, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ~
    0x00, 0x10, 0x28, 0x44, 0x82, 0x82, 0xFE, 0x00  // 
};

const int ScreenWidth =  224;
const int ScreenHeight = 288;

static TMachineInfo TestMachineInfo = { 
    "tickle", "Tickle", "http://www.ascotti.org/", 2014, ScreenWidth, ScreenHeight, 256, 60
};

bool TTickleMachine::initialize( TMachineDriverInfo * info )
{
    return false;
}

bool TTickleMachine::setResourceFile( int id, const unsigned char * buf, unsigned len )
{
    return false;
}

TTickleMachine::TTickleMachine()
{
    TMemoryInputStream mis( data, sizeof(data) );
    TBitmap * b = createBitmapFromPNG( &mis );

    assert( b != 0 );
    assert( b->format() == bfRGB );

    splash_ = reinterpret_cast<TBitmapRGB *>( b );

    screen_ = new TBitmapRGB( TestMachineInfo.screenWidth, TestMachineInfo.screenHeight );
    font_ = new TBitmapRGB( 8, 128*8 );
    font_->clear();

    int i;

    for( i=0; i<128; i++ ) {
        for( int y=0; y<8; y++ ) {
            unsigned b = fontdata[ i*8 + y ];

            for( int x=0; x<8; x++ ) {
                if( b & (1 << x) ) {
                    font_->setPixel( 7-x, i*8+y, 0xFFFFFF );
                }
            }
        }
    }

    screen_->clear();

    last_device_ = 0;
    dip_switches_ = 0;
    status_ = 0;

    registerDriver( TestMachineInfo );
}

TTickleMachine::~TTickleMachine()
{
    delete font_;
    delete splash_;
    delete screen_;
}

bool TTickleMachine::handleInputEvent( unsigned device, unsigned param, void * data )
{
    last_device_ = device;
    last_device_param_ = param;

    // Toggle mode if P1Start+P1Action1+CoinSlot1 pressed
    bool check = true;

    switch( device ) {
    case idKeyStartPlayer1:
        if( param & 1 ) status_ |= stP1StartPressed; else status_ &= ~stP1StartPressed;
        break;
    case idKeyP1Action1:
        if( param & 1 ) status_ |= stP1Action1Pressed; else status_ &= ~stP1Action1Pressed;
        break;
    case idCoinSlot1:
        if( param & 1 ) status_ |= stCoin1Pressed; else status_ &= ~stCoin1Pressed;
        break;
    default:
        check = false;
        break;
    }

    if( check && ((status_ & (stCoin1Pressed|stP1Action1Pressed|stP1StartPressed)) == (stCoin1Pressed|stP1Action1Pressed|stP1StartPressed)) ) {
        status_ ^= stTestMode;
    }

    return option_handler_.handleInputEvent( device, param );
}

void TTickleMachine::drawChar( int x, int y, char c )
{
    int index = (int)(unsigned char)c;
    TBltCopy blitter;

    unsigned * dst = screen_->data() + y * ScreenWidth + x;
    unsigned * src = font_->data() + index * 64;
    
    for( int i=8; i>0; i-- ) {
        memcpy( dst, src, 8*sizeof(unsigned) );
        src += 8;
        dst += ScreenWidth;
    }
}

void TTickleMachine::drawString( int x, int y, const char * s )
{
    if( x == -1 ) {
        x = (ScreenWidth - (int) strlen(s)*8) / 2;
    }

    while( *s != '\0' ) {
        drawChar( x, y, 0x7F & *s++ );
        x += 8;
    }
}

const char * getNameOf( unsigned n )
{
    static struct { 
        unsigned n; 
        const char * s; 
    } info[] = {
        { dcButton,         "button" },
        { dcTrigger,        "trigger" },
        { dcSwitch,         "switch" },
        { dcJoystick,       "joystick" },
        { dcOption,         "option" },
        { dtKey,            "generic" },
        { dtKeyAction,      "action" },
        { dtKeyStartPlayer, "start player" },
        { dtKeyService,     "service" },
        { dtTrigger,        "generic" },
        { dtCoinSlot,       "coin slot" },
        { dtSwitch,         "generic" },
        { dtJoystick,       "joystick" },
        { dtTrackball,      "trackball" },
        { dtMouse,          "mouse" },
        { dtLightGun,       "light gun" },
        { dtDipSwitch,      "DIP switch" },
        { dtDriverOption,   "driver configuration" },
        { dtCheat,          "cheat" },
        { 0, 0 }
    };

    const char * result = "n/a";

    for( int i=0; info[i].s; i++ ) {
        if( info[i].n == n ) {
            result = info[i].s;
            break;
        }
    }

    return result;
}    

void TTickleMachine::run( TFrame * frame, unsigned samplesPerFrame, unsigned samplingRate )
{
    if( status_ & stTestMode ) {
        screen_->clear();

        drawString( -1,  8, "TICKLE" );

        if( last_device_ == 0 ) {
            drawString( -1, 24, "Press any key or" );
            drawString( -1, 36, "use an input device" );
        }
        else {
            char buf[100];
            int y = 52;

            sprintf( buf, "DIP switches = %02X", dip_switches_ );
            drawString( 8, y, buf );
            y += 12;
        
            sprintf( buf, "device=%08Xh", last_device_ );
            drawString( 8, y, buf );
            y += 12;

            sprintf( buf, "param=%08Xh", last_device_param_ );
            drawString( 8, y, buf );
            y += 12;
            
            sprintf( buf, "class=%s", getNameOf( TInput::getDeviceClass(last_device_) ) );
            drawString( 8, y, buf );
            y += 12;

            sprintf( buf, "type=%s", getNameOf( TInput::getDeviceType(last_device_) ) );
            drawString( 8, y, buf );
            y += 12;
            
            sprintf( buf, "id=%d", TInput::getDeviceId(last_device_) & 0xFF );
            drawString( 8, y, buf );
            y += 12;
            
            switch( TInput::getDeviceClass(last_device_) ) {
            case dcButton:
                sprintf( buf, last_device_param_ & 1 ? "pressed" : "released" );
                break;
            case dcTrigger:
                sprintf( buf, last_device_param_ & 1 ? "active" : "inactive" );
                break;
            case dcSwitch:
                sprintf( buf, last_device_param_ & 1 ? "on" : "off" );
                break;
            case dcJoystick:
                sprintf( buf, "x = %d, y = %d", TInput::getXPosFromParam(last_device_param_), TInput::getYPosFromParam(last_device_param_) );
                break;
            case dcOption:
                sprintf( buf, "selection = %d", last_device_param_ );
                break;
            default:
                strcpy( buf, "" );
                break;
            }
            
            drawString( 8, y, buf );
            y += 12;
        }

        frame->setVideo( screen_ );
    }
    else {
        frame->setVideo( splash_ );
    }
}