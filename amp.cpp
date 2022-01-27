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
*  File name     : amp.cpp
*  System name   : jack_module
*
*  Description   : In->out copy with Volume control using JACK module
*
*
*  Author        : Marc_G
*  E-mail        : marc.groenewegen@hku.nl
*
**********************************************************************/

#include <iostream>
#include <string>
#include <math.h>
#include <thread>
#include <unistd.h> // sleep
#include "jack_module.h"
#include "keypress.h"

float amp=0.5; // NB: may produce high volume!


/*
 * With this abstraction module we don't need to know JACK's buffer size
 *   but we can independently determine our own block sizes
 */
unsigned long chunksize=2;


JackModule jack;
unsigned long samplerate=44100; // default



/*
 * filter function reads audio samples from JACK and writes a processed
 *   version back to JACK
 */
static void filter()
{
float *inbuffer = new float[chunksize];
float *outbuffer = new float[chunksize];


  do {
    jack.readSamples(inbuffer,chunksize);
    for(unsigned int x=0; x<chunksize; x++)
    {
      outbuffer[x]= amp * inbuffer[x];
    }
    jack.writeSamples(outbuffer,chunksize);
  } while(true);

} // filter()



int main(int argc,char **argv)
{
char command='@';

  jack.init(argv[0]); // use program name as JACK client name
  jack.autoConnect();

  samplerate=jack.getSamplerate();
  std::cerr << "Samplerate: " << samplerate << std::endl;

  std::thread filterThread(filter);

  init_keypress();

  while(command != 'q')
  {
  if(keypressed())
    {
      command = getchar();
      if(command == '+') amp *= 1.1;
      if(command == '-') amp *= 0.9;
      std::cout << "amp " << amp << std::endl;
    }
    usleep(100000);
  }

  filterThread.join();

  jack.end();

  return 0;
} // main()

