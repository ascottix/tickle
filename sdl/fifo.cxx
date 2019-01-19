/*
 Tickle class library
 FIFO queue for audio and video streams
 
 Copyright (c) 2014 Alessandro Scotti
 http://www.ascotti.org/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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


