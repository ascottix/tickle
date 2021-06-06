/*
    Tickle class library

    Copyright (c) 2003,2004 Alessandro Scotti
*/
#ifndef EMU_INITPROXY_H_
#define EMU_INITPROXY_H_

typedef void (* TInitFunction)( void * );

class TInitProxy
{
public:
    TInitProxy( TInitFunction onInit, TInitFunction onTerm = 0, void * param = 0 ) : init_func_(onInit), term_func_(onTerm), param_(param) {
        if( init_func_ != 0 ) {
            init_func_( param_ );
        }
    }

    /** 
        Destructor. 
        
        Invokes the user defined termination function.
    */
    ~TInitProxy() {
        if( term_func_ != 0 ) {
            term_func_( param_ );
        }
    }

private:
    TInitFunction init_func_;
    TInitFunction term_func_;
    void * param_;
};

#endif // EMU_INITPROXY_H_
