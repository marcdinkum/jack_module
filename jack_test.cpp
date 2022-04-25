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
*  File name     : jack_test.cpp
*  System name   : jack_module
* 
*  Description   : Test program for JACK module
*
*
*  Author        : Marc_G
*  E-mail        : marc.groenewegen@hku.nl
*
**********************************************************************/


#include <iostream>
#include <thread>
#include "jack_module.h"
#include "math.h"
#include "unistd.h"

/*
 * With this abstraction module we don't need to know JACK's buffer size
 *   but we can independently determine our own block sizes
 *
 * If anachunksize > synthchunksize we force buffer overflows
 * likewise, if anachunksize < synthchunksize we force buffer underruns
 */

unsigned long anachunksize=2048;
unsigned long synthchunksize=1024;



JackModule jack;

unsigned long samplerate=44100; // default

/*
 * play function generates audio samples and writes these to JACK
 */
static void play(unsigned long nframes)
{
float *synthbuffer = new float [synthchunksize];
unsigned nplayed=0;
double phase=0;

  do {

    // generate a chunk of samples
    for(unsigned long i=0; i<synthchunksize; i++){
      synthbuffer[i]=0.4*sin(phase);
      phase += 880*2*M_PI/(double)samplerate;
    } // for

    jack.writeSamples(synthbuffer,synthchunksize);
    nplayed+=synthchunksize;
  } while(nplayed < nframes);

  delete [] synthbuffer;

} // play()



/*
 * analysis function reads audio samples from JACK for further processing
 */
static void analysis(unsigned long nframes)
{
float *anabuffer = new float [anachunksize];
unsigned nread=0;

  do {
    jack.readSamples(anabuffer,anachunksize);

    // write incoming audio as text to stdout
    for(unsigned long frame=0; frame<anachunksize; frame++){
      std::cout << anabuffer[frame] << std::endl;
    }

    nread+=anachunksize;
  } while(nread < nframes);

  delete [] anabuffer;

} // analysis()



int main(int argc,char **argv)
{

  jack.init(argv[0]); // use program name as JACK client name
  jack.autoConnect();

  samplerate=jack.getSamplerate();
  std::cerr << "Samplerate: " << samplerate << std::endl;

  std::thread playThread(play,samplerate*5);
  std::thread analysisThread(analysis,samplerate*5);

  playThread.join();
  analysisThread.join();

  jack.end();

  return 0;
} // main()

