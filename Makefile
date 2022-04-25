# Makefile for jack module
#

# make install copies all necessary files to $INSTALL_DIR/lib/jack_module
INSTALL_DIR=/usr/local/lib/jack_module

CPP = g++ --std=c++11
CFLAGS = -Wall
LDFLAGS= -lpthread -ljack

RINGBUFOBJ = ringbuffer.o ringbuffer_test.o
ATOMICOBJ = atomic_test.o
JACKOBJ = ringbuffer.o jack_module.o jack_test.o

all: ringbuffer_test atomic_test jack_test

# mkdir -p : no error if already exists & make intermediate directories

install:
	sudo mkdir -p $(INSTALL_DIR)
	sudo cp jack_module.h jack_module.o ringbuffer.h ringbuffer.o $(INSTALL_DIR)



ringbuffer_test: $(RINGBUFOBJ)
	$(CPP) -o $@ $(CFLAGS) $(RINGBUFOBJ) $(LDFLAGS)

atomic_test: $(ATOMICOBJ)
	$(CPP) -o $@ $(CFLAGS) $(ATOMICOBJ)

jack_test: $(JACKOBJ)
	$(CPP) -o $@ $(CFLAGS) $(JACKOBJ) $(LDFLAGS)


.cpp.o:
	$(CPP) -c $< $(CFLAGS)

clean:
	rm -f *.o
	rm -f `find . -perm /111 -type f`

