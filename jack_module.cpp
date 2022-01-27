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
#include <mutex>
#include <unistd.h> // usleep

#include "jack_module.h"

// prototypes & globals
static void jack_shutdown(void *);
static jack_port_t *input_port,*output_port;

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


int JackModule::_wrap_jack_process_cb(jack_nframes_t nframes,void *arg)
{
  return ((JackModule *)arg)->onProcess(nframes);
} // _wrap_jack_process_cb()


int JackModule::onProcess(jack_nframes_t nframes)
{
  /* in and out are seen from JACK's perspective
   * meaning that
   *    (audio input)
   *  - samples are read from the input and written into the input ringbuffer
   *
   *    (audio output)
   *  - samples are read from the output ringbuffer and written to the output
   */
  void *in = jack_port_get_buffer(input_port,nframes);
  void *out = jack_port_get_buffer(output_port,nframes);

  // push input samples from JACK to the input ringbuffer
  frames_pushed=inputringbuffer->push((jack_default_audio_sample_t *)in,nframes);
  if(frames_pushed<nframes) std::cout << "Buffer full\n";

  // pop samples from output ringbuffer into JACK
  frames_popped=outputringbuffer->pop((jack_default_audio_sample_t *)out,nframes);
  if(frames_popped<nframes) std::cout << "Buffer empty\n";

  return 0;
} // onProcess()


int JackModule::init()
{
  return init("JackModule");
} // init()


int JackModule::init(std::string clientName)
{
  /*
   * Initialise a new client session
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

  output_port =
    jack_port_register(client,"output",JACK_DEFAULT_AUDIO_TYPE,JackPortIsOutput,0);
  input_port =
    jack_port_register(client,"input",JACK_DEFAULT_AUDIO_TYPE,JackPortIsInput,0);

  if(jack_activate(client)) {
    std::cout << "cannot activate client" << std::endl;
    return -1;
  } // if

  return 0;
} // init()


unsigned long JackModule::getSamplerate()
{
  return jack_get_sample_rate(client);
} // getSamplerate()


void JackModule::autoConnect()
{
  /*
   * Try to auto-connect our output
   *
   * NB: JACK considers reading from an output and sending to an input
   */
  if((ports = jack_get_ports(client,"system",NULL,JackPortIsInput)) == NULL)
  {
    std::cout << "Cannot find any physical output ports" << std::endl;
    exit(1);
  }

  if(jack_connect(client,jack_port_name(output_port),ports[0]))
  {
    std::cout << "Cannot connect output ports" << std::endl;
  }

  if(jack_connect(client,jack_port_name(output_port),ports[1]))
  {
    std::cout << "Cannot connect output ports" << std::endl;
  }

  free(ports); // ports structure no longer needed

  /*
   * Try to auto-connect our input
   */
  if((ports = jack_get_ports(client,NULL,NULL,JackPortIsPhysical|JackPortIsOutput)) == NULL)
  {
    std::cout << "Cannot find any physical capture ports" << std::endl;
    exit(1);
  }

  if(jack_connect(client,ports[0],jack_port_name(input_port)))
  {
    std::cout << "Cannot connect input ports" << std::endl;
  }

  free(ports); // ports structure no longer needed
} // autoConnect()


void JackModule::end()
{
  jack_deactivate(client);
  jack_port_disconnect(client,input_port);
  jack_port_disconnect(client,output_port);
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


