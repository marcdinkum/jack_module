/**********************************************************************
*          Copyright (c) 2017, Hogeschool voor de Kunsten Utrecht
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
*  File name     : ringbuffer.h
*  System name   : jack_module
* 
*  Description   : ring buffer class description
*		   Supports atomic read and write pointer updates and
*		    blocking and non-blocking modes
*
*
*  Author        : Marc_G
*  E-mail        : marc.groenewegen@hku.nl
*
**********************************************************************/



/*
 * ringbuffer.h
 */

#include <atomic>
#include <string>

class RingBuffer
{
public:
  RingBuffer(unsigned long size,std::string name);
  ~RingBuffer();
  unsigned long push(float *data,unsigned long n);
  unsigned long pop(float *data,unsigned long n);
  unsigned long items_available_for_write();
  unsigned long items_available_for_read();
  bool isLockFree();
  void pushMayBlock(bool block);
  void popMayBlock(bool block);
  void setBlockingNap(unsigned long blockingNap);
private:
  unsigned long size;
  float *buffer;
  std::atomic<unsigned long> tail; // write pointer
  std::atomic<unsigned long> head; // read pointer
  unsigned long itemsize; // also depends on #channels
  std::string name;
  bool blockingPush;
  bool blockingPop;
  unsigned long blockingNap=500;
}; // RingBuffer{}

