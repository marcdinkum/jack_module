# jack_module

C++ wrapper for JACK audio, containg [lock free ringbuffers](ringbuffer.md)
and some examples.

JACK Audio Connection Kit (or JACK) is a professional sound server API and
pair of daemon implementations to provide real-time, low-latency
connections for both audio and MIDI data between applications.

https://jackaudio.org

JACK provides a C API based on a callback model. This jack_module wrapper
offers a C++ class and hides the callback mechanism behind easy-to-use
read and write calls.


## Usage:

    include jack_module.h


Create a jack object:

    JackModule jack;

Optionally indicate the number of inputs and outputs. If you don't specify
them, the default is stereo: 2 inputs and 2 outputs.

    jack.setNumberOfInputChannels(2);
    jack.setNumberOfOutputChannels(2);

Initialise your JACK session. This creates the needed input- and output
ports but does not connect them.
Give your client a name within the JACK realm.

    jack.init("SuperSynth");

or

    jack.init(argv[0]); // use program name as JACK client name


If you want your client to automatically connect to ports of other
clients use

    jack.autoConnect();

This assumes that physical ports on your system are part of a client
named "system". If you want your client to connect to clients with another
name, give the names of the input and output clients as parameters to this
function. The first parameter (input) is the client that serves as a source
to our program, the second parameter (output) is the client we want to send
audio data to.

N.B.: Make sure the clients you connect to have enough ports available to
accommodate your setting of input- and output-channels (see above).

    jack.autoConnect("mplayer","system");

N.B. : this is case sensitive


Create the functionality of your program, using the input(s) and output(s)
you need. Samples are read from or handed to JACK as streams of
channel-interleaved sample frames, so if you use more than one channel you
will need to (de)interleave the streams.

Example of a loop that reads samples from a mono signal, does some processing
and writes the result as an interleaved stereo signal back to JACK:

    jack.readSamples(inbuffer,chunksize);
    for(unsigned int x=0; x<chunksize; x++)
    {
      // ... your algoritm here
      outbuffer[2*x]= left_sample;
      outbuffer[2*x+1]= right_sample;
    }
    jack.writeSamples(outbuffer,chunksize*2);

