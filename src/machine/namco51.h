/*
    Namco 51xx custom chip emulator

    Copyright (c) 2011 Alessandro Scotti
*/
#ifndef NAMCO_51XX_
#define NAMCO_51XX_

struct Namco5xCustom
{
    virtual unsigned char read() = 0;
    virtual void write( unsigned char b ) = 0;
};

struct Namco51xx : public Namco5xCustom
{
    Namco51xx( unsigned char * port0, unsigned char * port1 );

    void reset();
    
    unsigned char read();
    void write( unsigned char b );
    
    int credits_;
    int coins_[2];
    int creds_per_coin_[2];
    int coins_per_cred_[2];
    int data_count_;
    int mode_;
    unsigned char * port0_;
    unsigned char * port1_;
    unsigned char o_port0_;
    
private:
    Namco51xx();
};

#endif // NAMCO_51XX_
