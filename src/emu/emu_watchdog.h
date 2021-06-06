/*
    Tickle class library

    Copyright (c) 1997-2003,2004 Alessandro Scotti
*/
#ifndef EMU_WATCHDOG_H_
#define EMU_WATCHDOG_H_

class TWatchDog
{
public:
    TWatchDog( int timeout = 0 ) : timeout_(timeout) {
        reset();
    }

    void setTimeout( int timeout ) {
        timeout_ = timeout;
    }

    int timeout() const {
        return timeout_;
    }

    bool tick( TMachine * machine ) {
        bool result = false;

        time_++;

        if( (timeout_ > 0) && (time_ > timeout_) ) {
            reset();
            machine->reset();
            result = true;
        }

        return result;
    }

    void reset() {
        time_ = 0;
    }

private:
    int time_;
    int timeout_;
};

#endif // EMU_WATCHDOG_H_
