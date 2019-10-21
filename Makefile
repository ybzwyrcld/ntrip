.PHONY: clean

CC=
CFLAGS=-Wall -std=c++11 -pthread

INC=-I./include
LDFLAGS= -lpthread


all: ntrip_client ntrip_caster ntrip_server


ntrip_caster: examples/ntrip_caster.o \
	src/ntrip_caster.o \
	src/ntrip_util.o
	$(CC)g++ $^ ${LDFLAGS} -o $@

ntrip_client: examples/ntrip_client.o \
	src/ntrip_client.o \
	src/ntrip_util.o
	$(CC)g++ $^ ${LDFLAGS} -o $@

ntrip_server: examples/ntrip_server.o \
	src/ntrip_server.o \
	src/ntrip_util.o
	$(CC)g++ $^ ${LDFLAGS} -o $@

%.o:%.cc
	$(CC)g++ $(CFLAGS) $(INC) $(LDFLAGS) -o $@ -c $<

install:
	$(CC)strip ntrip_caster ntrip_client ntrip_server


clean:
	rm -rf src/*.o examples/*.o 
	rm -rf ntrip_caster ntrip_client ntrip_server

