.PHONY: clean

CC=
CFLAGS=-Wall -std=c++11 -pthread

INC=-I./include
LDFLAGS= -lpthread


all: ntrip_client


ntrip_client: examples/ntrip_client.o \
	src/client.o \
	src/util.o
	$(CC)g++ $^ ${LDFLAGS} -o $@


%.o:%.cc
	$(CC)g++ $(CFLAGS) $(INC) $(LDFLAGS) -o $@ -c $<


install:
	$(CC)strip ntrip_client 


clean:
	rm -rf src/*.o examples/*.o 
	rm -rf ntrip_client

