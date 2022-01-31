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
*  File name     : jack_module.cpp
*  System name   : jack_module
* 
*  Description   : C++ abstraction for JACK Audio Connection Kit
*		   Supports non-callback mode
*
*
*  Author        : Marc_G
*  E-mail        : marc.groenewegen@hku.nl
*
**********************************************************************/


#include <iostream>
#include <sstream>
#include <mutex>
#include <unistd.h> // usleep

#include "jack_module.h"

// prototypes & globals
static void jack_shutdown(void *);

/* ring buffer size depends partly on memory contraints but e.g.
 *  the tail of incoming audio to process may be a reason to request
 *  larger chunks, which is made easier by using a large ringbuffer
 */
#define DEFAULT_INRINGBUFSIZE 30000
#define DEFAULT_OUTRINGBUFSIZE 30000


JackModule::JackModule()
{
  inputringbuffer = new RingBuffer(DEFAULT_INRINGBUFSIZE,"in"); // audio in
  inputringbuffer->popMayBlock(true);
  inputringbuffer->setBlockingNap(500); // usec
  outputringbuffer = new RingBuffer(DEFAULT_OUTRINGBUFSIZE,"out"); // audio out
  outputringbuffer->pushMayBlock(true);
  outputringbuffer->setBlockingNap(500); // usec
} // JackModule()


JackModule::JackModule(unsigned long inbufsize, unsigned long outbufsize)
{
  inputringbuffer = new RingBuffer(inbufsize,"in"); // audio in
  inputringbuffer->popMayBlock(true);
  inputringbuffer->setBlockingNap(500); // usec
  outputringbuffer = new RingBuffer(outbufsize,"out"); // audio out
  outputringbuffer->pushMayBlock(true);
  outputringbuffer->setBlockingNap(500); // usec
} // JackModule()


JackModule::~JackModule()
{
  end();
} // ~JackModule()



int JackModule::init()
{
  return init("JackModule");
} // init()


int JackModule::init(std::string clientName)
{
  /*
   * Initialise a new client session and create the needed input and
   * output ports without connecting them.
   *
   * clientName: name of this client in the JACK connection overview
   * JackNoStartServer : do not try to start the server
   * JackNullOption or (jack_options_t)0 to try starting the
   *   server if it's not already running
   */

  if( (client=jack_client_open(clientName.c_str(),JackNoStartServer,NULL)) == 0) {
    std::cout << "JACK server not running ?" << std::endl;
    return 1;
  }

  // Install the callback wrapper and shutdown routine
  jack_on_shutdown(client,jack_shutdown,0); // install a shutdown callback
  jack_set_process_callback(client,_wrap_jack_process_cb,this);

  char inportname[8]="input_0"; // "input_" + 1-digit number + zero

  // create an array of -channel- jack_port_t elements
  input_port = new jack_port_t*[numberOfInputChannels];
  for(int channel=0; channel<numberOfInputChannels; channel++){
    inportname[6] = channel+0x31;
    input_port[channel] =
      jack_port_register(client,inportname,JACK_DEFAULT_AUDIO_TYPE,JackPortIsInput,0);
  }

  char outportname[9]="output_0"; // "output_" + 1-digit number + zero

  // create an array of -channel- jack_port_t elements
  output_port = new jack_port_t*[numberOfOutputChannels];
  for(int channel=0; channel<numberOfOutputChannels; channel++){
    outportname[7] = channel+0x31;
    output_port[channel] =
      jack_port_register(client,outportname,JACK_DEFAULT_AUDIO_TYPE,JackPortIsOutput,0);
  }

  // create buffer arrays of void pointers to prevent memory allocation inside
  //  the process loop
  inputbuffer = new jack_default_audio_sample_t*[numberOfInputChannels];
  outputbuffer = new jack_default_audio_sample_t*[numberOfOutputChannels];

  if(jack_activate(client)) {
    std::cout << "cannot activate client" << std::endl;
    return -1;
  } // if

  return 0;
} // init()





int JackModule::_wrap_jack_process_cb(jack_nframes_t nframes,void *arg)
{
  return ((JackModule *)arg)->onProcess(nframes);
} // _wrap_jack_process_cb()


/*
 * onProcess() gets called by JACK when it needs samples or has samples available
 *
 * This function handles reading samples from the input ringbuffer and
 * writing samples into the output ringbuffer.
 *
 * The client application supplies samples as interleaved sample frames to
 * the output ringbuffer, which are de-interleaved here for writing to the
 * JACK output(s).
 * 
 * JACK input(s) supply non-interleaved channels which are interleaved here
 * for writing into the input ringbuffer.
 *
 * The client application needs to make sure that the output buffers do not
 * run dry and the input buffers do not overflow.
 *
 */
int JackModule::onProcess(jack_nframes_t nframes)
{
  /* 
   *    (audio input)
   *  - samples are read from the input and written into the input ringbuffer
   *
   *    (audio output)
   *  - samples are read from the output ringbuffer and written to the output
   */

  // for each input port, get a buffer containing samples
  for(int channel=0; channel<numberOfInputChannels; channel++){
    inputbuffer[channel] = (jack_default_audio_sample_t *) jack_port_get_buffer(input_port[channel],nframes);
  }

  // for each output port, get a buffer for us to fill
  for(int channel=0; channel<numberOfOutputChannels; channel++){
    outputbuffer[channel] = (jack_default_audio_sample_t *) jack_port_get_buffer(output_port[channel],nframes);
  }

  // push input samples from JACK channel buffers to the input ringbuffer
  // interleave the samples before writing them to the ringbuffer

  int ringbufferindex=0;
  for(int frame=0; frame<(int)nframes; frame++){
    for(int channel=0; channel<numberOfInputChannels; channel++){
      tempbuffer[ringbufferindex]=inputbuffer[channel][frame];
      ringbufferindex++;
    }
  }

  frames_pushed=inputringbuffer->push((jack_default_audio_sample_t *)tempbuffer,nframes*numberOfInputChannels);
  if(frames_pushed<nframes*numberOfInputChannels) std::cout << "Buffer full\n";


  // pop samples from output ringbuffer into JACK channel buffers
  // de-interleave the samples from the ringbuffer and write them to the
  // appropriate JACK output buffers

  frames_popped=outputringbuffer->pop((jack_default_audio_sample_t *)tempbuffer,nframes*numberOfOutputChannels);
  if(frames_popped<nframes*numberOfOutputChannels) std::cout << "Buffer empty\n";

  ringbufferindex=0;
  for(int frame=0; frame<(int)nframes; frame++){
    for(int channel=0; channel<numberOfOutputChannels; channel++){
      outputbuffer[channel][frame]=tempbuffer[ringbufferindex];
      ringbufferindex++;
    }
  }

  return 0;
} // onProcess()


/*
 * 
 */
int JackModule::setNumberOfInputChannels(int n)
{
  if(n <= MAXINPUTCHANNELS){
    numberOfInputChannels=n;
    return 0;
  }
  else return -1;
}


int JackModule::setNumberOfOutputChannels(int n)
{
  if(n <= MAXOUTPUTCHANNELS){
    numberOfOutputChannels=n;
    return 0;
  }
  else return -1;
}


unsigned long JackModule::getSamplerate()
{
  return jack_get_sample_rate(client);
} // getSamplerate()


void JackModule::autoConnect()
{
  /*
   * NB: JACK considers reading from an output and sending to an input
   */

  /*
   * Try to auto-connect our output to the input of another client
   */
  if((ports = jack_get_ports(client,"system",NULL,JackPortIsInput)) == NULL)
  {
    std::cout << "Cannot find any physical output ports" << std::endl;
    exit(1);
  }

  if(jack_connect(client,jack_port_name(output_port[0]),ports[0]))
  {
    std::cout << "Cannot connect output ports" << std::endl;
  }

  if(jack_connect(client,jack_port_name(output_port[1]),ports[1]))
  {
    std::cout << "Cannot connect output ports" << std::endl;
  }

  free(ports); // ports structure no longer needed


  /*
   * Try to auto-connect our input to the output of another client
   */
  if((ports = jack_get_ports(client,NULL,NULL,JackPortIsPhysical|JackPortIsOutput)) == NULL)
  {
    std::cout << "Cannot find any physical capture ports" << std::endl;
    exit(1);
  }

  if(jack_connect(client,ports[0],jack_port_name(input_port[0])))
  {
    std::cout << "Cannot connect input ports" << std::endl;
  }

  free(ports); // ports structure no longer needed
} // autoConnect()


void JackModule::end()
{
  jack_deactivate(client);
  jack_port_disconnect(client,input_port[0]);
  jack_port_disconnect(client,output_port[0]);
} // end()


unsigned long JackModule::readSamples(float *ptr,unsigned long nrofsamples)
{
  // pop samples from JACK inputbuffer and hand over to the caller
  return inputringbuffer->pop(ptr,nrofsamples);
} // readSamples()


unsigned long JackModule::writeSamples(float *ptr,unsigned long nrofsamples)
{
  // push samples from the caller to the JACK outputbuffer
  return outputringbuffer->push(ptr,nrofsamples);
} // readSamples()


/*
 * shutdown callback may be called by JACK
 */
static void jack_shutdown(void *arg)
{
  exit(1);
} // jack_shutdown()


