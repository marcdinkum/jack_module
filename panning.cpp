/**********************************************************************
*          Copyright (c) 2022, Hogeschool voor de Kunsten Utrecht
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
*  File name     : panning.cpp
*  System name   : jack_module
*
*  Description   : example of stereo panning where a mono input signal is
*                    amplitude-panned between left and right outputs
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

float panFreq=1.0; // LFO frequency
double panPhase=0; // start in the center
float amp_left=0.5 * (sin(panPhase) + 1);
float amp_right=0.5 *(-sin(panPhase) + 1);

/*
 * With this abstraction module we don't need to know JACK's buffer size.
 *  We can determine our own block sizes and even let the input block
 *  size differ from the output block size as long as the total average
 *  input matches the total average output across the connected input- and
 *  output-channels.
 */
unsigned long chunksize=256;


JackModule jack;
unsigned long samplerate=44100; // default

bool running=true;


/*
 * filter function reads audio samples from JACK and writes a processed
 *   version back to JACK
 * Output is handed to JACK as interleaved sample frames
 */
static void filter()
{
float *inbuffer = new float[chunksize];
float *outbuffer = new float[chunksize*2];
float fader=0; // panning fader with range [-1,1]

  do {
    jack.readSamples(inbuffer,chunksize);
    for(unsigned int x=0; x<chunksize; x++)
    {
      fader = sin(panPhase);
      amp_left=0.5 * (fader + 1);
      amp_right=0.5 *(-fader + 1);
      outbuffer[2*x]= amp_left * inbuffer[x];
      outbuffer[2*x+1]= amp_right * inbuffer[x];
      panPhase += 2*M_PI*panFreq/samplerate;
    }
    jack.writeSamples(outbuffer,chunksize*2);
  } while(running);

  delete [] inbuffer;
  delete [] outbuffer;

} // filter()



int main(int argc,char **argv)
{
char command='@';

  jack.setNumberOfInputChannels(1);
  jack.setNumberOfOutputChannels(2);

  jack.init(argv[0]); // use program name as JACK client name

  jack.autoConnect();

  samplerate=jack.getSamplerate();
  std::cerr << "Samplerate: " << samplerate << std::endl;

  std::thread filterThread(filter);

  init_keypress();

  while(command != 'q')
  {
    if(keypressed()) {
      command = getchar();
      /*
       * '+' increases the panning rate with 10%
       *     for convenience the '=' key does the same as it's the
       *     same key without shift
       *
       * '-' decreases the panning rate with 10%
       */
      if(command == '+' || command == '=') panFreq *= 1.1;
      if(command == '-') panFreq *= 0.9;
      std::cout << "Panning frequency: " << panFreq << std::endl;
    }
    usleep(100000);
  }

  running=false;
  filterThread.join();

  jack.end();

  return 0;
} // main()

