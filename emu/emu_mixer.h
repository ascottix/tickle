/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
    http://www.ascotti.org/

    Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.
*/
#ifndef EMU_MIXER_H_
#define EMU_MIXER_H_

enum TMixerChannel 
{
    chMono = 0,
    chStereoLeft = 1,
    chStereoRight = 2
};

class TMixerBuffer
{
public:
    TMixerBuffer() : data_(0), size_(0), num_voices_(0) {
    }

    /** Destructor. */
    virtual ~TMixerBuffer() {
        delete data_;
    }

    virtual void clear();

    int * data() {
        return data_;
    }

    unsigned size() const {
        return size_;
    }

    unsigned voices() const {
        return num_voices_;
    }

    void setVoices( unsigned voices ) {
        num_voices_ = voices;
    }

    void addVoices( unsigned voices ) {
        num_voices_ += voices;
    }

    void expand( unsigned size );

private:
    int * data_;
    unsigned size_;
    unsigned num_voices_;
};

class TMixer
{
public:
    TMixer( unsigned channels );

    /** Destructor. */
    virtual ~TMixer();

    virtual void clear();

    virtual TMixerBuffer * getBuffer( unsigned channel, unsigned length, unsigned voices );

    const int * buffer( unsigned channel ) {
        return getBuffer( channel, 0, 0 )->data();
    }

    virtual int maxVoicesPerChannel();

    static void mix( int * dest, const int * source, unsigned len ) {
        while( len-- > 0 ) {
            *dest++ += *source++;
        }
    }

protected:
    virtual unsigned channels() const {
        return num_channels_;
    }

private:
    unsigned num_channels_;
    TMixerBuffer * buffers_;
};

class TMixerMono : public TMixer
{
public:
    TMixerMono() : TMixer(1) {
    }

    virtual TMixerBuffer * getBuffer( unsigned channel, unsigned length, unsigned voices );
};

class TMixerStereo : public TMixer
{
public:
    TMixerStereo() : TMixer(3) {
    }

    virtual TMixerBuffer * getBuffer( unsigned channel, unsigned length, unsigned voices );
};

#endif // EMU_MIXER_H_
