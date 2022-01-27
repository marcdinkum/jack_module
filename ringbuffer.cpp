/**********************************************************************
*          Copyright (c) 2018, Hogeschool voor de Kunsten Utrecht
*                      Hilversum, the Netherlands
*                          All rights reserved
***********************************************************************
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.
*  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************
*
*  File name     : ringbuffer.cpp
*  System name   : jack_module
* 
*  Description   : ring buffer class implementation
*		   Supports atomic read and write pointer updates and
*		    blocking and non-blocking modes
*
*
*  Author        : Marc_G
*  E-mail        : marc.groenewegen@hku.nl
*
**********************************************************************/



/*
 * Block-free ringbuffer for synchronisation of producer and consumer threads.
 *
 * Audio callback delivers data, the other thread reads data.
 *
 * Size must be a multiple of the real-time buffer size (e.g. JACK buffer).
 * For the non-realtime thread this is not an issue as long as it's large
 * enough to hold at least two consumer frames (theoretical minimum)
 *
 * Caveats: if consumer threads waits too long, producer can overrun the
 * buffer. This may not be a problem, for this we have resync() that puts
 * the consumer readpointer right on top of the consumer writepointer.
 */

#include <iostream>
#include "ringbuffer.h"
#include <unistd.h>
#include <string.h> // memcpy


 /*
  * Size is specified as #items, not bytes. Item type is now float and will
  * eventually be set in template form
  */
RingBuffer::RingBuffer(unsigned long size,std::string name)
{
  tail=0; // write pointer
  head=0; // read pointer
  this->size=size;
  itemsize=sizeof(float);
  buffer = new float [size]; // allocate storage
  this->name=name;
  // some defaults
  blockingPush=false;
  blockingPop=false;
  blockingNap=500;
} // RingBuffer()


RingBuffer::~RingBuffer()
{
  delete [] buffer;
} // ~RingBuffer()


unsigned long RingBuffer::items_available_for_write()
{
long pointerspace=(long)head.load()-(long)tail.load(); // signed

  if(pointerspace > 0) return pointerspace; // NB: > 0 so NOT including 0
  else return (unsigned long) (pointerspace+size);
} // items_available_for_write()


unsigned long RingBuffer::items_available_for_read()
{
long pointerspace=(long)tail.load()-(long)head.load(); // signed

  if(pointerspace >= 0) return pointerspace; // NB: >= 0 so including 0
  else return (unsigned long) (pointerspace+size);
} // items_available_for_read()


void RingBuffer::pushMayBlock(bool block)
{
  this->blockingPush=block;
} // pushMayBlock()


void RingBuffer::popMayBlock(bool block)
{
  this->blockingPop=block;
} // popMayBlock()


void RingBuffer::setBlockingNap(unsigned long blockingNap)
{
  this->blockingNap=blockingNap;
} // setBlockingNap()


/*
 * Try to write as many items as possible and return the number actually written
 */
unsigned long RingBuffer::push(float *data,unsigned long n)
{
  unsigned long space=items_available_for_write();

  if(blockingPush){
    // block and keep re-assessing available space
    while((space=items_available_for_write())<n){
      usleep(blockingNap);
    } // while
  } // if
  if(space<n) return 0; // reject partial chunks

  const auto current_tail = tail.load();
  if(current_tail + n <= (unsigned long)size){ // chunk fits without wrapping
    memcpy(buffer+current_tail,data,n*itemsize);
  }
  else {
    unsigned long first_chunk=size-current_tail;
    memcpy(buffer+current_tail,data,first_chunk*itemsize);
    memcpy(buffer,data+first_chunk,(n-first_chunk)*itemsize);
  }
  tail.store((current_tail+n)%size);
  return n;
} // push()


/*
 * Try to read as many items as possible and return the number actually read
 */
unsigned long RingBuffer::pop(float *data,unsigned long n)
{
  unsigned long space=items_available_for_read();

  if(blockingPop){
    while((space=items_available_for_read())<n){ // blocking
      usleep(blockingNap);
    } // while
  } // if
  if(space<n) return 0; // reject partial chunks

  const auto current_head = head.load();
  if(current_head + n <= (unsigned long)size){ // no wrapping necessary
    memcpy(data,buffer+current_head,n*itemsize);
  }
  else {
    unsigned long first_chunk=size-current_head;
    memcpy(data,buffer+current_head,first_chunk*itemsize);
    memcpy(data+first_chunk,buffer,(n-first_chunk)*itemsize);
  }
  head.store((current_head+n)%size);
  return n;
} // pop()


bool RingBuffer::isLockFree()
{
  return (tail.is_lock_free() && head.is_lock_free());
} // isLockFree()

