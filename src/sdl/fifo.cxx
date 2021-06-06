/*
    Tickle class library
    FIFO queue for audio and video streams
 
    Copyright (c) 2014 Alessandro Scotti
*/
#include "fifo.h"

struct FifoItem {
    void * data;
    FifoItem * next;
};

Fifo::Fifo() {
    this->head_ = 0;
    this->tail_ = 0;
}
    
Fifo::~Fifo() {
}
    
void Fifo::append( void * data ) {
    FifoItem * item = new FifoItem();
    
    item->data = data;
    item->next = 0;
    
    if( this->tail_ != 0 ) this->tail_->next = item;
    if( this->head_ == 0 ) this->head_ = item;
    
    this->tail_ = item;
}
    
void * Fifo::remove() {
    void * data = 0;
    
    if( this->head_ != 0 ) {
        FifoItem * item = this->head_;
        
        data = item->data;
        
        this->head_ = item->next;
        if( this->head_ == 0 ) this->tail_ = 0;
        
        delete item;
    }
    
    return data;
}
    
bool Fifo::empty() {
    return (this->head_ == 0);
}
    
int Fifo::count() {
    int result = 0;
    FifoItem * item = this->head_;
    
    while( item != 0 ) {
        result++;
        item = item->next;
    }
    
    return result;
}


