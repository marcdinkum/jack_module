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
*  File name     : atomic_test.cpp
*  System name   : jack_module
* 
*  Description   : Test program for atomic operations
*
*
*  Author        : Marc_G
*  E-mail        : marc.groenewegen@hku.nl
*
**********************************************************************/


#include <iostream>
#include <atomic>

int main()
{
  std::atomic <unsigned long> atomicvar;
  atomicvar.store(42);

  if(atomicvar.is_lock_free()) std::cout << "Lock free\n";
  else std::cout << "Not lock free\n";

  unsigned long tempvar = atomicvar.load();
  std::cout << "Value was " << tempvar << std::endl;

  return 0;
} // main()

