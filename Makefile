# Makefile for ringbuffer
#
CPP = g++ --std=c++11
CFLAGS = -Wall
LDFLAGS= -lpthread -ljack


JACKOBJ = ringbuffer.o jack_module.o jack_test.o
FXOBJ = ringbuffer.o jack_module.o
RINGBUFOBJ = ringbuffer.o ringbuffer_test.o
HOPOBJ = hopbuffer.o hopbuffer_test.o
ATOMICOBJ = atomic_test.o

all: ringbuffer_test jack_test atomic_test flanger fuzz echo delay


jack_test: $(JACKOBJ)
	$(CPP) -o $@ $(CFLAGS) $(JACKOBJ) $(LDFLAGS)

flanger: $(FXOBJ) flanger.o
	$(CPP) -o $@ $(CFLAGS) $(FXOBJ) flanger.o $(LDFLAGS)

fuzz: $(FXOBJ) fuzz.o
	$(CPP) -o $@ $(CFLAGS) $(FXOBJ) fuzz.o $(LDFLAGS)

echo: $(FXOBJ) echo.o
	$(CPP) -o $@ $(CFLAGS) $(FXOBJ) echo.o $(LDFLAGS)

delay: $(FXOBJ) delay.o
	$(CPP) -o $@ $(CFLAGS) $(FXOBJ) delay.o $(LDFLAGS)

ringbuffer_test: $(RINGBUFOBJ)
	$(CPP) -o $@ $(CFLAGS) $(RINGBUFOBJ) $(LDFLAGS)

atomic_test: $(ATOMICOBJ)
	$(CPP) -o $@ $(CFLAGS) $(ATOMICOBJ)

.cpp.o:
	$(CPP) -c $< $(CFLAGS)

clean:
	rm -f *.o
	rm -f `find . -perm /111 -type f`

