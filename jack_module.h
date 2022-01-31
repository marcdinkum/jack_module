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
*  File name     : jack_module.h
*  System name   : jack_module
* 
*  Description   : C++ abstraction for JACK Audio Connection Kit
*
*
*  Author        : Marc_G
*  E-mail        : marc.groenewegen@hku.nl
*
**********************************************************************/


#include <string>
#include <jack/jack.h>
#include "ringbuffer.h"

#define MAXINPUTCHANNELS 2
#define MAXOUTPUTCHANNELS 2


class JackModule
{
public:
  JackModule();
  JackModule(unsigned long inbufsize, unsigned long outbufsize);
  ~JackModule();
  int setNumberOfInputChannels(int n);
  int setNumberOfOutputChannels(int n);
  int init();
  int init(std::string clientName);
  unsigned long getSamplerate();
  void autoConnect();
  unsigned long readSamples(float *,unsigned long);
  unsigned long writeSamples(float *,unsigned long);
  void end();
private:
  static int _wrap_jack_process_cb(jack_nframes_t nframes,void *arg);
  jack_port_t **input_port;
  jack_port_t **output_port;
  jack_default_audio_sample_t **inputbuffer;
  jack_default_audio_sample_t **outputbuffer;
  jack_default_audio_sample_t tempbuffer[10000]; // FIXME get actual size
  int onProcess(jack_nframes_t nframes);
  int numberOfInputChannels=2;
  int numberOfOutputChannels=2;
  jack_client_t *client;
  const char **ports;
  RingBuffer *inputringbuffer; // jack writes into
  RingBuffer *outputringbuffer; // jack reads from
  unsigned long frames_pushed;
  unsigned long frames_popped;
};

