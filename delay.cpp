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
*  File name     : delay.cpp
*  System name   : jack_module
*
*  Description   : Feedback delay using JACK module
*
*
*  Author        : Marc_G
*  E-mail        : marc.groenewegen@hku.nl
*
**********************************************************************/

#include <iostream>
#include <string>
#include <math.h>
#include <unistd.h> // usleep
#include <thread>
#include "keypress.h"
#include "jack_module.h"


/*
 * With this abstraction module we don't need to know JACK's buffer size
 *   but we can independently determine our own block sizes
 */
unsigned long chunksize=2048;


JackModule jack;
unsigned long samplerate=44100; // default


/*
 * Delay line settings
 */
#define MAXDELAY 480000

jack_default_audio_sample_t delayline[MAXDELAY+1]; // one extra location!
unsigned long delay=MAXDELAY;
unsigned long delay_index=0;

bool running=true;

/*
 * dryWet range [0,1] where:
 *   1 -> fully dry, meaning the input signal is passed without modification
 *   0 -> fully wet, meaning the output signal is only the processed
 *        version of the input signal without mixing in the original input
 */
float dryWet=0.5; // default: half dry, half wet


/*
 * feedBack specifies the amount of delayed signal added to the input
 * signal before entering the delay line.
 *
 * Allowed range: [0,1] where:
 *  0 -> no feedback, meaning the output is just delayed
 *  1 -> every sample leaving the delay line is added to the input sample
 *       without attenuation, which may get out of hand quickly
 */
float feedBack=0.7;


/*
 * filter function reads audio samples from JACK and writes a processed
 *   version back to JACK
 *
 * Be careful not to mix up interleaved channels
 */
static void filter()
{
float *inbuffer = new float[chunksize];
float *outbuffer = new float[chunksize];
float drySample;

  do {
    jack.readSamples(inbuffer,chunksize);
    for(unsigned int x=0; x<chunksize; x++)
    {
      drySample = dryWet * inbuffer[x]; // store it for later use
      outbuffer[x] = (1-dryWet) * delayline[delay_index];
      // write a new sample into delay line
      delayline[delay_index]=inbuffer[x] + feedBack * outbuffer[x];
      outbuffer[x] += drySample; // add dry signal
      delay_index++;
      delay_index%=delay;
    }
    jack.writeSamples(outbuffer,chunksize);
  } while(running);

} // filter()



int main(int argc,char **argv)
{
char command='@';

  jack.setNumberOfInputChannels(2);
  jack.setNumberOfOutputChannels(2);

  jack.init(argv[0]); // use program name as JACK client name
  jack.autoConnect();

  samplerate=jack.getSamplerate();
  std::cerr << "Samplerate: " << samplerate << std::endl;

  std::thread filterThread(filter);
  std::cout << "Delay " << (float)delay/samplerate << " sec" << std::endl;

  init_keypress();

  while(running)
  {
   if(keypressed()) {
      command = getchar();
      /*
       * 'w' increases dryWet with 5% with a max of 1.0
       * 'W' decreases dryWet with 5% with a min of 0.0
       */
      if(command == 'w') dryWet *= 1.05; // dryer
      if(dryWet > 1.0) dryWet=1.0;
      if(command == 'W') dryWet *= 0.95; // wetter
      if(dryWet < 0) dryWet=0;
      std::cout << "Dry/wet " << dryWet << std::endl;

      /*
       * 'F' increases feedBack with 5% with a max of 1.0
       * 'f' decreases feedBack with 5% with a min of 0.0
       */
      if(command == 'F') feedBack *= 1.05;
      if(feedBack > 1.0) feedBack=1.0;
      if(command == 'f') feedBack *= 0.95;
      if(feedBack < 0) feedBack=0;
      std::cout << "Feedback " << feedBack << std::endl;

      /*
       * 'D' increases delay with 10% with a max of MAXDELAY
       * 'd' decreases delay with 10% with a min of 1
       */
      if(command == 'D') delay *= 1.1;
      if(delay%2 != 0) ++delay; // don't mix up (interleaved) left and right
      if(delay > MAXDELAY) delay=MAXDELAY;
      /*
       * when increasing the delay, clear rest of delay line as it may contain
       * data from previous runs
       */

      if(command == 'd') delay *= 0.9;
      if(delay%2 != 0) ++delay; // don't mix up (interleaved) left and right
      if(delay < 0) delay=0;
      std::cout << "Delay " << (float)delay/samplerate << " sec" << std::endl;
    }
    usleep(100000);

  } // while running

  filterThread.join();

  jack.end();

  return 0;
} // main()

