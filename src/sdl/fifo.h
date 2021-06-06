/*
    Tickle class library
 
    Copyright (c) 2014 Alessandro Scotti
*/
#ifndef __tickle__fifo__
#define __tickle__fifo__

struct FifoItem;

class Fifo {
private:
    FifoItem * head_;
    FifoItem * tail_;
    
public:
    Fifo();
    
    virtual ~Fifo();
    
    void append( void * item );
    
    void * remove();
    
    bool empty();
    
    int count();
};

#endif /* defined(__tickle__fifo__) */
