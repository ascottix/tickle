#ifndef EMU_FLAGS_H_
#define EMU_FLAGS_H_

class TFlags 
{
public:
    TFlags( unsigned value = 0 ) : value_(value) {
    }

    bool test( unsigned bit ) {
        return (value_ & bit) != 0;
    }

    bool testBits( unsigned bits ) {
        return (value_ & bits) == bits;
    }

    void set( unsigned bit ) {
        value_ |= bit;
    }

    void clear( unsigned bit ) {
        value_ &= ~bit;
    }

    void clear() {
        value_ = 0;
    }

    void set( unsigned bit, int condition ) {
        if( condition ) {
            set( bit );
        }
        else {
            clear( bit );
        }
    }
           
private:
    unsigned value_;
};

#endif // EMU_FLAGS_H_
