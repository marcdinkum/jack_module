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
*  File name     : ringbuffertest.cpp
*  System name   : jack_module
* 
*  Description   : ring buffer test
*
*
*  Author        : Marc_G
*  E-mail        : marc.groenewegen@hku.nl
*
**********************************************************************/



#include <iostream>
#include "ringbuffer.h"


int main()
{
RingBuffer buffer(10,"Buffer");
float inputdata[8]={1,2,3,4,5,6,7,8};
float anadata[8];

  if(buffer.isLockFree()) std::cout << "Lock free\n";
  else std::cout << "Not lock free\n";

  std::cout << "Avail for write: " << buffer.items_available_for_write() << std::endl;
  std::cout << "Avail for read: " << buffer.items_available_for_read() << std::endl;
  buffer.push(inputdata,1);
  buffer.push(inputdata,5);
    buffer.pop(anadata,5);
  std::cout << "Avail for write: " << buffer.items_available_for_write() << std::endl;
  std::cout << "Avail for read: " << buffer.items_available_for_read() << std::endl;

  buffer.push(inputdata,5);
  std::cout << "Avail for write: " << buffer.items_available_for_write() << std::endl;
  std::cout << "Avail for read: " << buffer.items_available_for_read() << std::endl;
  buffer.push(inputdata,1);
  std::cout << "Avail for write: " << buffer.items_available_for_write() << std::endl;
  std::cout << "Avail for read: " << buffer.items_available_for_read() << std::endl;
  buffer.push(inputdata,1);
  std::cout << "Avail for write: " << buffer.items_available_for_write() << std::endl;
  std::cout << "Avail for read: " << buffer.items_available_for_read() << std::endl;

  if(buffer.items_available_for_read() >= 5){
    buffer.pop(anadata,5);
  std::cout << "Avail for write: " << buffer.items_available_for_write() << std::endl;
  std::cout << "Avail for read: " << buffer.items_available_for_read() << std::endl;
    for(unsigned long i=0; i<5; i++) std::cout << anadata[i] << " ";
  }
  else std::cout << "Not enough data" << std::endl;

  buffer.push(inputdata,1);
  std::cout << "Avail for write: " << buffer.items_available_for_write() << std::endl;
  std::cout << "Avail for read: " << buffer.items_available_for_read() << std::endl;
  buffer.push(inputdata,1);
  std::cout << "Avail for write: " << buffer.items_available_for_write() << std::endl;
  std::cout << "Avail for read: " << buffer.items_available_for_read() << std::endl;

  if(buffer.items_available_for_read() >= 5){
    buffer.pop(anadata,5);
  std::cout << "Avail for write: " << buffer.items_available_for_write() << std::endl;
  std::cout << "Avail for read: " << buffer.items_available_for_read() << std::endl;
    for(unsigned long i=0; i<5; i++) std::cout << anadata[i] << " ";
  }
  else std::cout << "Not enough data" << std::endl;

  std::cout << std::endl;
  return 0;
}

